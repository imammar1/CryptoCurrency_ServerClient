/*
* Author(s)    : Parashar Parikh,
*				 Ibrahim Ammar,
*				 Zeineb Moalla
* File         : Implementation file for the server class methods
* Date created : 09/29/2022
* Date modified: 11/10/2022
*/

#include "tcp_server.hpp"

database sq_database;
int clients = 0;
bool server_shutdown = false;

// Error handler for write
void handler::handle_write(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (error && quit_received == false) {
        cout << "Write failed: %s\n" << error.message().c_str();
        quit();
    }
    else if (quit_received)
    {
        quit();
    }
    else if (server_shutdown) 
    {
        shutdown();
    }
}

// Handler for the read the data 
void handler::handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
{
    // Check if the client closed connection
    if (error && quit_received == false) {
        printf("Received failed: %s \n", error.message().c_str()); // Error handling
        return;
    }

    // Get the ip of the remote endpoint
    string remote_ip = socket_.remote_endpoint().address().to_string();

    string msg(received_data);
    received_message = msg + " ";
    printf("Received message from Client: %s \n", received_data);

    // Process the database request
    //int* ptr = &userid;
    sent_message = sq_database.process(received_message, remote_ip, &userid);

    // Check if the message is asking to us to shutdown or quit
    string token = received_message.substr(0, received_message.find(" "));
    std::transform(token.begin(), token.end(), token.begin(), toupper);

    // Will return the message that will be sent to the user.
    if (token == "SHUTDOWN" && userid != -1) 
    {
        server_shutdown = true;
    }
    else if (token == "QUIT")
    {
        quit_received = true;
    }

    if (!error)
    {
        // Clear the read buffer before reading 
        memset(received_data, 0, sizeof received_data);
        start();
    }
    
}

    void handler::start()
    {

        // only read if quit command is issued 
        if (quit_received == false)
            socket_.async_read_some(
                boost::asio::buffer(received_data, max_length),
                boost::bind(&handler::handle_receive,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
            
        //// To serve the data to the client.
        boost::asio::async_write(socket_,
            boost::asio::buffer(sent_message, max_length),
            boost::bind(&handler::handle_write,
                shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

    }

    void handler::quit()
    {
        quit_received = true;
        clients--;
        socket_.close();

    }

// Closes socket for all users upon shutdown
void handler::shutdown()
{
    quit_received = true;
    server_shutdown = true;
    clients = 0;
    socket_.close();
    exit(300);
}

// Constructor initialises an acceptor to listen on TCP port 3211
Server::Server(boost::asio::io_context& io_context) : io(io_context),
con_acceptor(io_context, tcp::endpoint(tcp::v4(), PORT))
{
    start_accept();
}

// Creates a socket and initiates an asynchronous 
// accept operation to wait for a new connection
void Server::start_accept()
{
    if (server_shutdown == false)
    {
        handler::pointer new_connection = handler::create(io);
        // Waits until client tries to connect
        con_acceptor.async_accept(new_connection->socket(),
            boost::bind(&Server::handle_accept, this, new_connection,
                boost::asio::placeholders::error));
    }
    else
        con_acceptor.close();

}

// Services the client request, and then calls 
    // start_accept() to initiate the next accept operation
void Server::handle_accept(handler::pointer new_connection,
    const boost::system::error_code& error)
{
    if (!error)
    {
        clients++;
        cout << "Number of connected clients: " << clients << endl;
        new_connection->start();
    }

    start_accept();
}
