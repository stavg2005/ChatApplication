#include "../include/client.hpp"
#include "../include/ConsoleUtils.hpp"
#include <boost/asio.hpp>
#include <iostream>


namespace net = boost::asio;
using     tcp = net::ip::tcp;

//──────────────── ctor / dtor ────────────────
Client::Client(net::io_context& io,
               std::string      host,
               unsigned short   port)
    : io_(io)
    , socket_(io)
    , resolver_(io)
    , host_(std::move(host))
    , port_(port)
{}

Client::~Client()
{
    running_ = false;
    if (input_thread_.joinable())
        input_thread_.join();
}

//──────────────── public API ──────────────────
void Client::start()
{
    resolver_.async_resolve(
        host_, std::to_string(port_),
        [self = shared_from_this()](auto ec, tcp::resolver::results_type eps)
        {
            if (ec) {
                std::cerr << "Resolve failed: " << ec.message() << '\n';
                return;
            }
            net::async_connect(
                self->socket_, eps,
                [self](auto ec2, tcp::endpoint)
                {
                    if (ec2) {
                        std::cerr << "Connect failed: "
                                  << ec2.message() << '\n';
                        return;
                    }
                    self->send_name();
                });
        });
}

//──────────────── private helpers … ───────────
void Client::send_name()
{
    std::cout << "Enter your name: ";
    std::getline(std::cin, name_);
    name_ += '\n';

    auto self = shared_from_this();
    net::async_write(socket_, net::buffer(name_),
        [self](auto ec, std::size_t)
        {
            if (ec) {
                std::cerr << "Send-name failed: " << ec.message() << '\n';
                return;
            }
            self->read_loop();
            self->launch_input_loop();
        });

    con::strip_trailing_newlines(name_);   // use helper from ConsoleUtils
}

void Client::write(std::string text)
{
    auto self   = shared_from_this();
    auto buffer = std::make_shared<std::string>(std::move(text));

    std::scoped_lock lk(write_mtx_);
    net::async_write(socket_, net::buffer(*buffer),
        [self, buffer](auto ec, std::size_t)
        {
            if (ec) {
                std::cerr << "Write failed: " << ec.message() << '\n';
                self->running_ = false;
            }
        });
}

void Client::read_loop()
{
    auto self = shared_from_this();
    net::async_read_until(socket_, resp_buf_, '\n',
        [self](const boost::system::error_code& ec, std::size_t)
        {
            if (ec) {        
                             // Ignore normal shutdown errors
                if (!(ec == net::error::eof ||
                      ec == net::error::operation_aborted ||
                      ec == net::error::connection_reset)) {
                    std::cerr << "Read failed: " << ec.message() << '\n'; // ← only log real errors
                }           
                    
                
                self->running_ = false;
                return;                                   
            }

            std::istream is(&self->resp_buf_);
            std::string line;
            std::getline(is, line);
            self->show_incoming(line);
            self->read_loop();                             
        });
}

void Client::show_incoming(std::string_view msg)
{
    con::erase_current_line();
    std::cout << msg << '\n'
              << '[' << name_ << "] " << std::flush;
}

void Client::launch_input_loop()
{
    input_thread_ = std::thread([self = shared_from_this()] {
        std::cout << '[' << self->name_ << "] " << std::flush;

        std::string line;
        while (self->running_ && std::getline(std::cin, line)) {
            if (line == "/quit") {
                self->running_ = false;
            }
            if (line.empty())
                continue;

            con::erase_previous_line(); // erase local echo
            line += '\n';
            net::post(self->io_, [self, msg = std::move(line)]() mutable {
                self->write(std::move(msg));
            });
        }
    });
}

void Client::send_quit() {
    if (!running_) return;
    running_ = false;

    write("/quit\n");  // send command to server


    boost::system::error_code ec;
    if (socket_.is_open()) {
        boost::system::error_code ec;
        if (auto rc = socket_.shutdown(tcp::socket::shutdown_both, ec);
            rc && rc != boost::asio::error::not_connected) {
        std::cerr << "shutdown failed: " << rc.message() << '\n';
    }

    if (auto rc = socket_.close(ec);
        rc && rc != boost::asio::error::not_connected) {
      std::cerr << "close failed: " << rc.message() << '\n';
    }
  }

    if (input_thread_.joinable())
        input_thread_.join();

    std::exit(0);
}




