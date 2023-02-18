#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

//! based on ASIO echo server tutorial

class session_t : public std::enable_shared_from_this<session_t> {
    void do_read();
    void do_write(std::size_t length);

    boost::asio::ip::tcp::socket m_socket;
    enum
    {
        max_length = 1024
    };
    char m_data[max_length];

    public:
    session_t(boost::asio::ip::tcp::socket socket);
    void start();
};

class server_t {
    void do_accept();

    boost::asio::ip::tcp::acceptor m_acceptor;
    public:

    server_t(boost::asio::io_context &io_context, int port);
};