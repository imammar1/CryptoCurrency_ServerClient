// Implementation for the client 

#include <iostream>
#include <ctime>
#include <string>
#include <cstring>
#include <stdint.h>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#define PORT 8610

using namespace std;
using boost::asio::ip::tcp;
using boost::asio::ip::address;

int main(int argc, char** argv)
{
    if (argc == 1) 
    {
        cout << "Please provide the ip of the client!! \n";
        return -1;
    }

    try {

        boost::asio::io_context io_context;
        // Socket creation
        tcp::socket socket(io_context);
        // Socket connection
        socket.connect(tcp::endpoint(boost::asio::ip::address::from_string(argv[1]), PORT));
        cout << "Connection established on " << PORT << endl;

        // Infinte loop until the connection is terminated
        for (;;)
        {
            boost::system::error_code write_error;
            string temp;
            std::getline(cin, temp);  // Getting input from user
            
            // Sends the messages to the server
            boost::asio::write(socket, boost::asio::buffer(temp), write_error);
            if (write_error) {
                cout << "send failed: " << write_error.message() << endl;
                throw boost::system::system_error(write_error);  // Some other error
            }


            boost::array<char, 1024> buf;
            boost::system::error_code error;

            size_t len = socket.read_some(boost::asio::buffer(buf), error);
            if (error == boost::asio::error::eof)
                break;     // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error);   // Some other error.

            cout.write(buf.data(), len);     // Prints out to stream
            cout << "---------" << endl;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;  // Exception handling
    }

    return 0;

}