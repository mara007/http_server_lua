#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <functional>

#include <boost/asio.hpp>

#include "http.hpp"

//! based on ASIO echo server tutorial
class connection_t : public std::enable_shared_from_this<connection_t> {
    public:
    using new_msg_cb_t = std::function<void(std::shared_ptr<connection_t>, std::shared_ptr<http_req_t>)>;

    connection_t(boost::asio::ip::tcp::socket socket);
    void register_new_msg_cb(new_msg_cb_t cb);
    void start();

    private:
    void do_read();
    void do_write(std::size_t length);

    enum  { max_length = 1024 };
    char m_data[max_length];
    http_buffer_t<max_length> m_http_buffer;
    boost::asio::ip::tcp::socket m_socket;
    new_msg_cb_t m_new_msg_cb;
    decltype(m_http_buffer)::http_msg_reasembled_cb_t m_new_msg_reassebled_cb;
};

class server_t {
    public:

    using new_conn_cb_t = std::function<void(std::shared_ptr<connection_t>)>;
    void register_new_conn_cb(new_conn_cb_t cb);
    server_t(boost::asio::io_context &io_context, int port);

    private:
    void do_accept();

    boost::asio::ip::tcp::acceptor m_acceptor;
    new_conn_cb_t m_new_conn_cb;
};