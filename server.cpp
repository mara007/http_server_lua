#include "server.h"
#include "http.hpp"
#include <boost/log/trivial.hpp>

/************* SESSION ****************/
session_t::session_t(boost::asio::ip::tcp::socket socket)
: m_socket(std::move(socket))
{}

void session_t::do_read() {
    auto self = shared_from_this();
    auto cb = [self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
            self->do_write(length);
        }
    };
    m_socket.async_read_some(boost::asio::buffer(m_data, max_length), cb);
}

void session_t::do_write(std::size_t length) {
    auto self = shared_from_this();
    auto cb = [self](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
            self->do_read();
        }
    };
    BOOST_LOG_TRIVIAL(info) << "======\ngot data:\n" << m_data << std::endl;
    boost::asio::async_write(m_socket, boost::asio::buffer(m_data, length), cb);
}

void session_t::start() {
    do_read();
}

/************* SERVER ****************/
server_t::server_t(boost::asio::io_context &io_context, int port)
: m_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
    do_accept();
}

void server_t::do_accept() {
    auto cb = [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
        if (!ec) {
            auto ep = socket.remote_endpoint();
            BOOST_LOG_TRIVIAL(info) << "New connection from " << ep.address().to_string();
            std::make_shared<session_t>(std::move(socket))->start();
        }

        do_accept();
    };
    m_acceptor.async_accept(cb);
}

