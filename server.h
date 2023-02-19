#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <functional>
#include <deque>
#include <vector>
#include <thread>

#include <boost/asio.hpp>

#include "http.hpp"

//! based on ASIO echo server tutorial
class http_conn_t : public std::enable_shared_from_this<http_conn_t> {
    public:
    using new_msg_cb_t = std::function<void(std::shared_ptr<http_conn_t>, std::shared_ptr<http_req_t>)>;

    http_conn_t(boost::asio::io_context& io_context, boost::asio::ip::tcp::socket socket);
    virtual ~http_conn_t();

    void register_new_msg_cb(new_msg_cb_t cb);
    void start();

    void send_response(http_resp_t resp);

    private:
    void do_read();
    void do_queue_message(std::string resp_str);
    void start_packet_send();

    enum  { max_length = 1024 };
    char m_socket_buf[max_length];
    http_buffer_t<max_length> m_http_buffer;
    std::deque<std::string> m_send_queue; //! serialized responses
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::io_context& m_io_context;
    boost::asio::io_context::strand m_io_write_strand;
    new_msg_cb_t m_new_msg_cb;
    decltype(m_http_buffer)::http_msg_reasembled_cb_t m_new_msg_reassebled_cb;
};

class server_t {
    public:

    using new_conn_cb_t = std::function<void(std::shared_ptr<http_conn_t>)>;

    server_t(size_t threads);
    void register_new_conn_cb(new_conn_cb_t cb);
    void start_server(unsigned port);

    private:
    void do_accept();

    size_t m_threads;
    std::vector<std::thread> m_thread_pool;
    boost::asio::io_context m_io_context;
    boost::asio::ip::tcp::acceptor m_acceptor;
    new_conn_cb_t m_new_conn_cb;
};