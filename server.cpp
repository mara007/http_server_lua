#include "server.h"
#include "http.hpp"
#include <boost/log/trivial.hpp>

/************* CONNECTION ****************/
http_conn_t::http_conn_t(boost::asio::io_context& io_context, boost::asio::ip::tcp::socket socket)
: m_http_buffer()
, m_socket(std::move(socket))
, m_io_context(io_context)
, m_io_write_strand(io_context)
{
    auto msg_cb = [this](std::shared_ptr<http_req_t> new_req) {
        if (m_new_msg_cb)
            m_new_msg_cb(shared_from_this(), new_req);
    };
    m_http_buffer.register_msg_reasembled_cb(msg_cb);
}

void http_conn_t::register_new_msg_cb(new_msg_cb_t cb) {
    m_new_msg_cb = cb;
}

http_conn_t::~http_conn_t() {
    BOOST_LOG_TRIVIAL(info) << "connection_t::~connection_t()";
}

void http_conn_t::do_read() {
    auto self = shared_from_this();
    auto cb = [self, this](boost::system::error_code ec, std::size_t length) {
        if (ec)
            return;

        auto res = m_http_buffer.new_data(m_socket_buf, length);
        if (!res) {
            // self->m_socket.shutdown(); //boost 1.81 
            auto resp = http_resp_t(400, "BAD REQUEST");
            resp.add_header("connection", "close");
            self->send_response(resp);
            BOOST_LOG_TRIVIAL(info) << "connection_t - closing socket due to data decoding error";
            self->m_socket.close();
        }

        do_read();
    };
    m_socket.async_read_some(boost::asio::buffer(m_socket_buf, max_length), cb);
}

void http_conn_t::start() {
    do_read();
}

void http_conn_t::send_response(http_resp_t resp) {
    BOOST_LOG_TRIVIAL(info) << "connection_t::send_response()";

    m_io_context.post(m_io_write_strand.wrap(
        [self=shared_from_this(), resp_str=resp.serialize_to_string()](){
            self->do_queue_message(resp_str);
        }));
    // auto resp_str = resp.serialize_to_string();
    // boost::system::error_code ec;
    // size_t sent_bytes = boost::asio::write(m_socket, boost::asio::buffer(resp_str), ec);

    // if (ec) {
    //     BOOST_LOG_TRIVIAL(error) << "error - can't send response: " << ec.message();
    //     return;
    // }
    // self->do_read();
}

void http_conn_t::do_queue_message(std::string resp_str) {
    //no locking - io_context::strand is used to ensure 'send' events are processed sequentially
    bool write_in_progress = !m_send_queue.empty();
    m_send_queue.push_back(std::move(resp_str));

    if (!write_in_progress) {
        start_packet_send();
    }
}

void http_conn_t::start_packet_send() {
    auto send_done_cb = [self=shared_from_this()](auto err, auto) {
        if (err)
            return;

        self->m_send_queue.pop_front();
        if (!self->m_send_queue.empty())
            self->start_packet_send();
    };
    boost::asio::async_write(m_socket, boost::asio::buffer(m_send_queue.front()),
                             m_io_write_strand.wrap(send_done_cb));

}
/************* SERVER ****************/
server_t::server_t(size_t threads)
: m_threads(threads), m_acceptor(m_io_context)
{}

void server_t::start_server(unsigned port) {
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

void server_t::do_accept() {
    auto cb = [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
        if (!ec) {
            auto ep = socket.remote_endpoint();
            BOOST_LOG_TRIVIAL(info) << "New connection from " << ep.address().to_string();

            auto conn = std::make_shared<http_conn_t>(m_io_context, std::move(socket));
            conn->start();
            if (m_new_conn_cb)
                m_new_conn_cb(conn);

            do_accept();
        }
    };
    m_acceptor.async_accept(cb);
}

void server_t::register_new_conn_cb(server_t::new_conn_cb_t cb) {
    m_new_conn_cb = cb;
}