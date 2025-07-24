#pragma once
#include <boost/asio.hpp>
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>

class Session;                      // forward
struct ClientSessionInfo;           // forward

namespace net = boost::asio;
using     tcp = net::ip::tcp;

/**
 * Chat server: accepts TCP clients, spawns a Session for each,
 * and broadcasts messages.
 */
class Server
{
public:
    explicit Server(net::io_context& io_context, short port);

private:
    // helpers
    int  generate_client_id();
    void on_client_identified(int id, const std::string& name);
    void on_client_message   (int id, const std::string& text);
    void do_accept();
    void on_client_disconnect(int id);
    void broadcast(const std::string& text);
    void broadcastNoEcho(int id,const std::string& payload);

    // data
    tcp::acceptor                                              acceptor_;
    std::unordered_map<int, std::shared_ptr<ClientSessionInfo>> clients_;
    std::deque<std::string>                                   recent_messages_;
};

/* ---------------------------------------------------------------------------
 * Helper wrapper stored in the map
 * -------------------------------------------------------------------------*/
struct ClientSessionInfo {
    std::shared_ptr<Session> session;
    std::string              name;
    explicit ClientSessionInfo(std::shared_ptr<Session> s)
        : session(std::move(s)) {}
};
