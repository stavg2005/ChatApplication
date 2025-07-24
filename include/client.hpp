#pragma once
//──────────────────────────────────────────────────────────────────────────────
// Client.hpp ― public interface for the asynchronous chat client
//
//   • Uses Boost.Asio for networking
//   • One instance owns a TCP socket, resolver, and a background input thread
//
// 2025-07-23
//──────────────────────────────────────────────────────────────────────────────
#include <boost/asio.hpp>
#include <atomic>
#include <mutex>
#include <string>
#include <thread>

namespace net = boost::asio;
using     tcp = net::ip::tcp;

/**
 * @brief  Asynchronous chat client.
 *
 * The object lives on the heap via `std::shared_ptr` and keeps itself alive
 * while outstanding asynchronous operations are pending (`enable_shared_from_this`).
 * After connecting it:
 *   • sends the user’s name,
 *   • launches an input thread to read from `std::cin`,
 *   • echoes incoming messages without wrecking the user’s current prompt.
 */
class Client : public std::enable_shared_from_this<Client>
{
public:
    /// Construct with an existing io_context, remote host, and port.
    Client(net::io_context& io,
           std::string      host,
           unsigned short   port);

    /// Joins the background input thread on destruction.
    ~Client();

    /// Begin async-resolve → async-connect → chat loops.
    void start();

    void send_quit();

    

private:
    //── networking helpers ──────────────────────────────────────────────
    void send_name();                   ///< prompt user & write the name line
    void write(std::string text);       ///< enqueue a chat line to the socket
    void read_loop();                   ///< perpetual async_read_until('\n')
    

    //── UI helpers ──────────────────────────────────────────────────────
    void show_incoming(std::string_view msg);  ///< pretty-print a server line
    void launch_input_loop();                  ///< spawn std::thread for stdin

    //── data members ────────────────────────────────────────────────────
    net::io_context&   io_;        ///< event loop (owned by caller)
    tcp::socket        socket_;    ///< connected after start()
    tcp::resolver      resolver_;  ///< for DNS / endpoint lookup
    net::streambuf     resp_buf_;  ///< collects bytes until '\n'

    std::mutex         write_mtx_; ///< serialize writes to the socket
    std::thread        input_thread_;

    std::string        host_;
    unsigned short     port_;
    std::string        name_;      ///< cached “clean” name (no trailing \n)
    std::atomic_bool   running_{true};
};
extern std::shared_ptr<Client> global_client;