#include "../include/Server.hpp"
#include <boost/asio.hpp>
#include <iostream>

int main()
{
    try {
        boost::asio::io_context io;
        Server srv(io, 12345);
        std::cout << "Server running on port 12345...\n";
        io.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
