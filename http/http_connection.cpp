#include "http_connection.h"
#include <boost/log/trivial.hpp>



http_connection_t::http_connection_t(boost::asio::io_context& io_context, boost::asio::ip::tcp::socket socket)
: abstract_connection_t(io_context, std::move(socket))
, m_http_buffer()
, m_io_write_strand(io_context)
{
    auto msg_cb = [this](std::shared_ptr<http_req_t> new_req) {
        if (m_new_msg_cb)
            m_new_msg_cb(shared_from_this(), new_req);
    };
    m_http_buffer.register_msg_reasembled_cb(msg_cb);
}

void http_connection_t::register_new_msg_cb(new_msg_cb_t cb) {
    m_new_msg_cb = cb;
}

http_connection_t::~http_connection_t() {
    BOOST_LOG_TRIVIAL(info) << "connection_t::~connection_t()";
}

void http_connection_t::do_read() {
    auto self = shared_from_this();
    auto cb = [self, this](boost::system::error_code ec, std::size_t length) {
        if (ec) {
            BOOST_LOG_TRIVIAL(error) << "connection_t::do_read() - in socket::async_read_some() CB - error: "
                                     << ec.message() << ". Closing socket";

            self->m_socket.close();
            return;
        }

        auto res = m_http_buffer.new_data(m_socket_buf, length);
        if (!res) {
            // self->m_socket.shutdown(); //boost 1.81 
            auto resp = http_resp_t(400, "BAD REQUEST");
            resp.add_header("connection", "close");
            self->send_response(resp);
            BOOST_LOG_TRIVIAL(info) << "connection_t - socket will be close due to a decoding error";
            m_io_context.post(m_io_write_strand.wrap([self]() {
                BOOST_LOG_TRIVIAL(info) << "connection_t - closing socket due to a data decoding error";
                self->m_socket.close();
            }));
        }

        do_read();
    };
    m_socket.async_read_some(boost::asio::buffer(m_socket_buf, max_length), cb);
}

void http_connection_t::start() {
    do_read();
}

void http_connection_t::send_response(http_resp_t& resp) {
    BOOST_LOG_TRIVIAL(info) << "connection_t::send_response()";

    m_io_context.post(m_io_write_strand.wrap(
        [self=shared_from_this(), resp_str=resp.serialize_to_string()](){
            self->do_queue_message(resp_str);
        }));
}

// 
// void http_connection_t::send_response_sync(http_resp_t resp) {
//     auto resp_str = resp.serialize_to_string();
//     boost::system::error_code ec;
//     boost::asio::write(m_socket, boost::asio::buffer(resp_str), ec);
//     if (ec) {
//         BOOST_LOG_TRIVIAL(error) << "error - can't send response: " << ec.message();
//         return;
//     }
// }

void http_connection_t::do_queue_message(std::string resp_str) {
    //no locking - io_context::strand is used to ensure 'send' events are processed sequentially
    bool write_in_progress = !m_send_queue.empty();
    m_send_queue.push_back(std::move(resp_str));

    if (!write_in_progress) {
        start_packet_send();
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "WRITE IN PROGRESS";
}

void http_connection_t::start_packet_send() {
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