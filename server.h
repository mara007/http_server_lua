#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;

class session_t : public std::enable_shared_from_this<session_t>
{
public:
    session_t(tcp::socket socket)
    : m_socket(std::move(socket))
    {}

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self = shared_from_this();
        auto cb = [self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                self->do_write(length);
            }
        };
        m_socket.async_read_some(boost::asio::buffer(m_data, max_length), cb);
    }

    void do_write(std::size_t length) {
        auto self = shared_from_this();
        auto cb = [self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                self->do_read();
            }
        };
        BOOST_LOG_TRIVIAL(info) << "======\ngot data:\n" << m_data << std::endl;
        boost::asio::async_write(m_socket, boost::asio::buffer(m_data, length), cb);
    }

    tcp::socket m_socket;
    enum
    {
        max_length = 1024
    };
    char m_data[max_length];
};

class server_t
{
public:
    server_t(boost::asio::io_context &io_context, int port)
    : m_acceptor(io_context, tcp::endpoint(tcp::v4(), port))
    {
        do_accept();
    }

private:
    void do_accept() {
        auto cb = [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                auto ep = socket.remote_endpoint();
                BOOST_LOG_TRIVIAL(info) << "New connection from " << ep.address().to_string();
                std::make_shared<session_t>(std::move(socket))->start();
            }

            do_accept();
        };
        m_acceptor.async_accept(cb);
    }

    tcp::acceptor m_acceptor;
};