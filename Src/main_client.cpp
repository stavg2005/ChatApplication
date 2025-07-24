// src/main_client.cpp
#include "../include/client.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <csignal>

void handle_sigint(int) {
    if (global_client) {
        std::cout << "\n[Client] Ctrl+C pressed. Exiting...\n";
        global_client->send_quit();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: client <host> <port>\n";
        return 1;
    }

    std::signal(SIGINT, handle_sigint);  // Set Ctrl+C handler

    net::io_context io;
    global_client = std::make_shared<Client>(
        io, argv[1], static_cast<unsigned short>(std::stoi(argv[2])));

    global_client->start();
    io.run();
}

