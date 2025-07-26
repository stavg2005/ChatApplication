#include "../include/Server.hpp"
#include "../include/session.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

Server::Server(net::io_context& io, short port)
    : acceptor_(io, tcp::endpoint(tcp::v4(), port))
{
    do_accept();
}

//──────────────── private helpers ──────────────
int Server::generate_client_id()
{
    int id = std::rand() % 1000;
    while (clients_.find(id) != clients_.end())
        id = std::rand() % 1000;
    return id;
}

void Server::on_client_identified(int id, const std::string& name)
{
    if (auto it = clients_.find(id); it != clients_.end()) {
        it->second->name = name;
        std::cout << "Client " << id << " is named " << name << '\n';
        std::string payload = "[" + clients_[id]->name + "] " + "Conected" + '\n';
        broadcastNoEcho(id,payload);
        if (!recent_messages_.empty()) {
            std::string backlog;
            for (auto& m : recent_messages_) backlog += m;
            it->second->session->deliver(backlog);
        }
    }
}

void Server::on_client_message(int id, const std::string& text)
{
    std::string sender = "Unknown";
    if (auto it = clients_.find(id); it != clients_.end() && !it->second->name.empty())
        sender = it->second->name;

    std::string payload = "[" + sender + "] " + text + '\n';
    broadcast(payload);
}

void Server::broadcast(const std::string& payload){
  recent_messages_.push_back(payload);
    for (auto& [_, info] : clients_)
        info->session->deliver(payload);
}

void Server::broadcastNoEcho(int id, const std::string& payload){
    recent_messages_.push_back(payload);
    for (auto& [client_id, entry] : clients_) { 
        if (client_id == id)                     // ← skip echo
            continue;
        entry->session->deliver(payload);   // broadcast
    }
        
}

void Server::on_client_disconnect(int id){
    if (auto it = clients_.find(id); it != clients_.end()){
      
      std::string payload = "[" + clients_[id]->name + "] " + "Dissconected" + '\n';
      std::cout << payload;
      broadcastNoEcho(id,payload);
      clients_.erase(it); 
    }
}

void Server::do_accept()
{
    acceptor_.async_accept(
        [this](auto ec, tcp::socket socket) {
            if (!ec) {
                int cid = generate_client_id();

                auto session = std::make_shared<Session>(
                    std::move(socket), cid,
                    [this](int i,const std::string& n){ on_client_identified(i,n); },
                    [this](int i,const std::string& m){ on_client_message(i,m); },
                    [this](int i){on_client_disconnect(i);}
                  );

                clients_[cid] = std::make_shared<ClientSessionInfo>(session);
                session->start();
            }
            do_accept();
        });
}
