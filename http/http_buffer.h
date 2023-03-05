#pragma once

#include "http_message.h"

#include <map>
#include <string>
#include <functional>
#include <algorithm>
#include <memory>
#include <optional>
#include <mutex>
#include <ostream>

#include <boost/log/trivial.hpp>

//! parses http messages from tcp stream data
template<size_t N>
class http_buffer_t {
    public:
    using http_msg_reasembled_cb_t = std::function<void(std::shared_ptr<http_req_t>)>;

    http_buffer_t()
    : m_buffer()
    , m_msg_reassembled_cb(nullptr)
    , m_http_msg_waiting_for_body(nullptr)
    , m_body_bytes_to_read(0)
    {
        m_buffer.reserve(N);
    }

    http_buffer_t(http_msg_reasembled_cb_t cb)
    : http_buffer_t()
    {
        m_msg_reassembled_cb = std::move(cb);
    }

    void register_msg_reasembled_cb(http_msg_reasembled_cb_t cb) {
        m_msg_reassembled_cb = std::move(cb);
    }

    enum new_data_state_e : uint8_t {
        NEED_MORE_DATA = 0,
        MSG_ERROR,
        MSG_OK
    };

    new_data_state_e new_data(const char* data, size_t length) {
        BOOST_LOG_TRIVIAL(debug) << "http_buffer_t::new_data() buffer.size=" << m_buffer.size()
                                 << ", buffer.capacity=" << m_buffer.capacity() << ", data length=" << length;
        if (m_buffer.size() + length > m_buffer.capacity())
            return new_data_state_e::MSG_ERROR;

        std::lock_guard guard(m_buff_mutex);
        std::copy(data, data+length, std::back_inserter(m_buffer));

        bool msg_finished = false;
        size_t http_msg_end;
        do {
            if (m_body_bytes_to_read) {
                assert(m_http_msg_waiting_for_body.operator bool());
                auto bytes_got = std::min(m_body_bytes_to_read, length);
                m_http_msg_waiting_for_body->body.append(m_buffer, 0, bytes_got);

                m_body_bytes_to_read -= bytes_got;

                if (m_body_bytes_to_read == 0) {
                    m_http_msg_waiting_for_body->parse_body();

                    if (m_msg_reassembled_cb) {
                        BOOST_LOG_TRIVIAL(info) << "reasebled with body callback: " << *m_http_msg_waiting_for_body;
                        m_msg_reassembled_cb(m_http_msg_waiting_for_body);
                    }
                    m_http_msg_waiting_for_body = nullptr;
                    m_buffer.erase(0, bytes_got);
                    msg_finished = true;
                    continue;
                }
            }
            http_msg_end = m_buffer.find(MSG_SEPARATOR);

            if (http_msg_end == std::string::npos) {
                continue;
            }

            auto http_msg = http_req_t::parse(m_buffer.data(), http_msg_end);

            if (!http_msg)
                return new_data_state_e::MSG_ERROR;

            m_buffer.erase(0, http_msg_end + MSG_SEPARATOR.size());

            if (auto content_length = http_msg->get_header("content-length"); content_length) {
                BOOST_LOG_TRIVIAL(debug) << "messages with 'content-length': " << content_length.value();
                size_t cl;
                try {
                    cl = std::stol(content_length.value());
                } catch (std::invalid_argument& e) {
                    BOOST_LOG_TRIVIAL(error) << "Invalid 'content-length'";
                    return new_data_state_e::MSG_ERROR;
                }

                if (cl > MAX_BODY_SIZE)
                    return new_data_state_e::MSG_ERROR;

                m_body_bytes_to_read = cl;
                m_http_msg_waiting_for_body = http_msg;
                continue;
            }

            if (m_msg_reassembled_cb) {
                BOOST_LOG_TRIVIAL(info) << "reasebled callback: " << *http_msg;
                m_msg_reassembled_cb(http_msg);
            }
            msg_finished = true;

            BOOST_LOG_TRIVIAL(info) << "ASSEMBLE: end_msg=" << http_msg_end << ", size=" << m_buffer.size();
        } while(http_msg_end != std::string::npos); //check if multiple messages arrived in one packet/data bulk

        return msg_finished ? new_data_state_e::MSG_OK : new_data_state_e::NEED_MORE_DATA;
    }

    private:
    std::string m_buffer;
    std::mutex m_buff_mutex;
    http_msg_reasembled_cb_t m_msg_reassembled_cb;

    std::shared_ptr<http_req_t> m_http_msg_waiting_for_body;
    size_t m_body_bytes_to_read;
};

