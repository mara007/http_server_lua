#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <type_traits>
#include <vector>


#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

#include "abstract_connection.h"

// based on ASIO echo server tutorial

template<typename connection_t>
class server_t {
    static_assert(std::is_base_of<abstract_connection_t, connection_t>::value,
                 "ERROR server_t<> template argument class must be derived from abstract_connection_t");

    public:

    using new_conn_cb_t = std::function<void(std::shared_ptr<connection_t>)>;

    server_t(size_t threads)
    : m_threads(threads), m_acceptor(m_io_context)
    {}

    void register_new_conn_cb(new_conn_cb_t cb){
        m_new_conn_cb = cb;
    }

    void start_server(unsigned port) {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
        m_acceptor.open(endpoint.protocol());
        m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

        m_acceptor.bind(endpoint);
        m_acceptor.listen();

        do_accept();

        boost::asio::signal_set signals(m_io_context, SIGINT);
        signals.async_wait([=](auto err, int sig_code){
            BOOST_LOG_TRIVIAL(warning) << "Signal " << sig_code << " cought, stopping!";
            m_io_context.stop();
        });

        for (auto i = 0; i < m_threads; ++i)
            m_thread_pool.emplace_back([=](){
                m_io_context.run();
                BOOST_LOG_TRIVIAL(info) << "Thread no. " << i << " stopping.";

            });

        for (auto& t: m_thread_pool)
            t.join();
    }

    private:
    void do_accept() {
        auto cb = [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (ec)
                return;

            auto ep = socket.remote_endpoint();
            BOOST_LOG_TRIVIAL(info) << "New connection from " << ep.address().to_string();

            auto conn = std::make_shared<connection_t>(m_io_context, std::move(socket));
            BOOST_LOG_TRIVIAL(info) << "New connection created";
            conn->start();
            if (m_new_conn_cb)
                m_new_conn_cb(conn);

            do_accept();
        };
        m_acceptor.async_accept(cb);
    };

    size_t m_threads;
    std::vector<std::thread> m_thread_pool;
    boost::asio::io_context m_io_context;
    boost::asio::ip::tcp::acceptor m_acceptor;
    new_conn_cb_t m_new_conn_cb;
};