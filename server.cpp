#include "server.h"
#include "http.hpp"
#include <boost/log/trivial.hpp>

/************* CONNECTION ****************/
connection_t::connection_t(boost::asio::ip::tcp::socket socket)
: m_http_buffer(), m_socket(std::move(socket))
{
    auto msg_cb = [this](std::shared_ptr<http_req_t> new_req) {
        if (m_new_msg_cb)
            m_new_msg_cb(shared_from_this(), new_req);
    };
    m_http_buffer.register_msg_reasembled_cb(msg_cb);
}

void connection_t::register_new_msg_cb(new_msg_cb_t cb) {
    m_new_msg_cb = cb;
}

void connection_t::do_read() {
    auto self = shared_from_this();
    auto cb = [self, this](boost::system::error_code ec, std::size_t length) {
        if (ec)
            return;

        auto res = m_http_buffer.new_data(m_data, length);
        if (!res) {
            // self->m_socket.shutdown(); //boost 1.81 
            auto resp = http_resp_t(400, "BAD REQUEST");
            resp.add_header("connection", "close");
            self->send_response(resp);
            BOOST_LOG_TRIVIAL(info) << "connection_t - closing socket due to data decoding error";
            self->m_socket.close();
        }

    };
    m_socket.async_read_some(boost::asio::buffer(m_data, max_length), cb);
}

void connection_t::do_write(std::size_t length) {
    auto self = shared_from_this();
    auto cb = [self](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
            self->do_read();
        }
    };
    boost::asio::async_write(m_socket, boost::asio::buffer(m_data, length), cb);
}

void connection_t::start() {
    do_read();
}

void connection_t::send_response(const http_resp_t& resp) {
    BOOST_LOG_TRIVIAL(info) << "connection_t::send_response()";

    auto self = shared_from_this();
    auto cb = [self](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
            self->do_read();
        }
    };

    auto resp_str = resp.serialize_to_string();
    auto resp_buff = boost::asio::buffer(resp_str);
    boost::system::error_code ec;
    size_t sent_bytes = boost::asio::write(m_socket, resp_buff, ec);

    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "error - can't send response: " << ec.message();
    }
    self->do_read();
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

            auto conn = std::make_shared<connection_t>(std::move(socket));
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