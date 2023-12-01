#include "tcp_server.hpp"

// Client code to test the server code
int main(int argc, char** argv)
{
    try
    {
        boost::asio::io_context io_context;
        Server server(io_context);
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}