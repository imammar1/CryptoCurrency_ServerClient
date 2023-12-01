#pragma once
/*
* Author(s)    : Parashar Parikh,
*				 Ibrahim Ammar,
*				 Zeineb Moalla
* File         : Header file for the tcp server class
* Date created : 09/29/2022
* Date modified: 11/20/2022
*/

#include "database.hpp"
#include <cstring>
#include <stdint.h>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::address;

#define PORT 8610

// This class is responsible for handling the 
// tcp connection which reads in the message and 
// sends the message back to the user
class handler : public boost::enable_shared_from_this<handler>
{
private:
    tcp::socket socket_;
    int userid = -1;    // User not logged in by default 
    string received_message;
    string sent_message;
    enum { max_length = 1024 };
    char received_data[max_length];
    bool quit_received = false;

    // Private functions for handling connection
    handler(boost::asio::io_context& io_context) : socket_(io_context) {};
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);
    void quit();
    void shutdown();

public:

    // To keep the tcp_connection object alive as long as 
    // there is an operation that refers to it
    typedef boost::shared_ptr<handler> pointer;
    // Return the pointer to the newly created connection
    static pointer create(boost::asio::io_context& io_context) { return pointer(new handler(io_context)); }
    tcp::socket& socket() { return socket_; }
    // Called to start the connection
    void start();
};


class Server
{
private:
    boost::asio::io_context& io;
    tcp::acceptor con_acceptor;
    void start_accept();
    void handle_accept(handler::pointer new_connection,
        const boost::system::error_code& error);

public:
    Server(boost::asio::io_context& io_context);

};
