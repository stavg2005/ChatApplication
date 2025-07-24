#pragma once
//──────────────────────────────────────────────────────────────────────────────
// Session.hpp ― represents one connected client on the server side
//
//   • Owns the TCP socket for that client
//   • Reads the user’s name (phase 1), then chat lines (phase 2)
//   • Relays incoming messages to the Server via callbacks
//   • Queues outbound messages so only one async_write is active at a time
//
// 2025‑07‑23
//──────────────────────────────────────────────────────────────────────────────
#include <boost/asio.hpp>
#include <deque>
#include <functional>
#include <memory>
#include <string>

namespace net = boost::asio;
using     tcp = net::ip::tcp;

/**
 * @brief  One per connected client; created by Server.
 *
 * Lifetime is managed with `std::shared_ptr` + `enable_shared_from_this` so the
 * object stays alive while any asynchronous operation is outstanding.
 */
class Session : public std::enable_shared_from_this<Session>
{
public:
    /// Callbacks the Session uses to talk back to Server
    using NameCallback = std::function<void(int, const std::string&)>; ///< id, name
    using MsgCallback  = std::function<void(int, const std::string&)>; ///< id, text
    using DiconnectCallBack = std::function<void(int)>;
    /**
     * @param socket   freshly‑accepted (already connected) socket
     * @param id       unique client identifier assigned by Server
     * @param name_cb  invoked once when the user’s name arrives
     * @param msg_cb   invoked for every subsequent chat message
     * @param dis_cp   invoked one when the users quits
     */
    Session(tcp::socket   socket,
            int           id,
            NameCallback  name_cb,
            MsgCallback   msg_cb,
            DiconnectCallBack dis_cb);

    /// Begin the read‑name phase; called immediately after construction.
    void start();

    /// Enqueue a message to be delivered to this client.
    void deliver(const std::string& msg);

    void stop();

private:
    //── phase 1: read client name ─────────────────────────────────────────
    void read_name();

    //── phase 2: chat mode ────────────────────────────────────────────────
    void do_read();   ///< async_read_until('\n') loop
    void do_write();  ///< write front of outbox_ then pop

    void print_incoming(const std::string& msg); ///< server console helper

    

    //── data members ──────────────────────────────────────────────────────
    tcp::socket            socket_;
    net::streambuf         streambuf_;
    int                    client_id_;
    NameCallback           name_callback_;
    MsgCallback            msg_callback_;
    DiconnectCallBack      dis_callback_;
    std::string            client_name_;   ///< cached after read_name()
    std::deque<std::string> outbox_;       ///< pending outbound messages
};
