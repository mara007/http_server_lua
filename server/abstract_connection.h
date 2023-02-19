#pragma once

#include <boost/asio.hpp>

//! connection handlers used with server_t<> must implement this interface
class abstract_connection_t {
    public:
    abstract_connection_t(boost::asio::io_context& io_context, boost::asio::ip::tcp::socket socket)
    : m_socket(std::move(socket)), m_io_context(io_context)
    {}

    virtual void start() = 0;

    protected:
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::io_context& m_io_context;
};