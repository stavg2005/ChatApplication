#include "../include/session.hpp"
#include <iostream>

Session::Session(tcp::socket socket, int id, NameCallback name_cb,
                 MsgCallback msg_cb, DiconnectCallBack dis_cb)
    : socket_(std::move(socket)), client_id_(id),
      name_callback_(std::move(name_cb)), msg_callback_(std::move(msg_cb)),
      dis_callback_(std::move(dis_cb)) {}

void Session::start() { read_name(); }

// ──────────────── phase 1 – name ───────────────
void Session::read_name() {
  auto self = shared_from_this();
  net::async_read_until(
      socket_, streambuf_, '\n', [this, self](auto ec, std::size_t length) {
        if (ec) {
          std::cerr << "Read name error: " << ec.message() << '\n';
          dis_callback_(client_id_);
          return;
        }
        std::istream is(&streambuf_);
        std::getline(is, client_name_);
        streambuf_.consume(length); // discard bytes
        name_callback_(client_id_, client_name_);
        do_read(); // switch to chat mode
      });
}

// ──────────────── phase 2 – chat loop ───────────
void Session::do_read() {
  auto self = shared_from_this();
    net::async_read_until(socket_, streambuf_, '\n',
        [this, self](const boost::system::error_code& ec, std::size_t /*len*/)
        {
            if (ec) {
                std::cerr << "[Session for client " <<  client_name_ << "] read failed: " << ec.message() << '\n';
                if (dis_callback_) dis_callback_(client_id_);
                return;
            }

            std::istream is(&streambuf_);
            std::string line;
            std::getline(is, line);

            if (line == "/quit") {                      // client wants out
                if (dis_callback_) dis_callback_(client_id_);
                stop();
                return;
            }
            msg_callback_(client_id_, line);
           
            

            do_read();   
        });
}
void Session::print_incoming(const std::string &msg) {
  std::cout << "\r\x1B[2K" << msg << std::flush;
}

// ──────────────── write queue ───────────────────
void Session::deliver(const std::string &msg) {

  bool in_progress = !outbox_.empty();
  outbox_.push_back(msg);
  //if there are still message that need to be printed before this one 
  if (!in_progress)
    do_write();
}

void Session::stop() {
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
}

void Session::do_write() {
  auto self = shared_from_this();
  net::async_write(socket_, net::buffer(outbox_.front()),
                   [this, self](auto ec, std::size_t) {
                     if (!ec) {
                       outbox_.pop_front();
                       if (!outbox_.empty())
                         do_write();
                     }
                   });
}
