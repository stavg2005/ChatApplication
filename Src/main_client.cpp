// src/main_client.cpp
#include "../include/client.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <csignal>


namespace {
    std::function<void()> cleanup_handler;

    void signal_handler(int) {
        if (cleanup_handler)
            cleanup_handler(); 
    }
}

int main(int argc, char* argv[]) {
      if (argc != 3) {
        std::cerr << "Usage: client <host> <port>\n";
        return 1;
    }

    net::io_context io;

    auto client = std::make_shared<Client>(
        io, argv[1], static_cast<unsigned short>(std::stoi(argv[2])));

    
    cleanup_handler = [client] {
        std::cout << "\n[Client] Ctrl+C pressed. Sending /quit and exiting...\n";
        client->send_quit();  
    };

    std::signal(SIGINT, signal_handler);  // Register Ctrl+C signal

    client->start();
    io.run();
}

