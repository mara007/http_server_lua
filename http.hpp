#pragma once

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <memory>
#include <optional>

#include <boost/log/trivial.hpp>

static const std::string MSG_SEPARATOR = "\r\n\r\n";
static const std::string NEW_LINE = "\r\n";

struct http_req_t {
    std::multimap<std::string, std::string> headers;
    std::string method;
    std::string path;
    std::string body;

    //! if multiple headers 'name' present, returns the first one
    std::optional<std::string> get_header(const std::string& name);
    void add_header(std::string name, std::string value);

    /*! parses give http message without the body - 'CLRF [ message-body ]' part 
     *  - header names will be in lowercase
     *  ABNF:
     *  - HTTP-message = start-line CRLF *( field-line CRLF ) CRLF [ message-body ]
     */
    static std::shared_ptr<http_req_t> parse(const char* data, size_t lenght);
};

template<size_t N>
class http_buffer_t {
    public:
    using http_msg_reasembled_cb_t = std::function<void(std::shared_ptr<http_req_t>)>;

    http_buffer_t()
    : m_buffer(), m_new_msg_cb(nullptr)
    {}

    http_buffer_t(http_msg_reasembled_cb_t cb)
    : m_buffer(), m_new_msg_cb(std::move(cb))
    {
        m_buffer.reserve(N);
        m_buff_pos = std::begin(m_buffer);
    }

    void register_msg_reasembled_cb(http_msg_reasembled_cb_t cb) {
        m_new_msg_cb = cb;
    }

    bool new_data(const char* data, size_t length) {
        auto curr_size = std::distance(std::begin(m_buffer), m_buff_pos);
        if (curr_size + length > m_buffer.capacity())
            return false;

        m_buff_pos = std::copy(data, data+length, m_buff_pos);

        typename decltype(m_buffer)::iterator http_msg_end;
        do {
            http_msg_end = std::find_end(std::begin(m_buffer), m_buff_pos,
                                        std::begin(MSG_SEPARATOR), std::end(MSG_SEPARATOR));

            // BOOST_LOG_TRIVIAL(info) << "MSG:\n" << std::string(m_buffer.begin(), m_buff_pos) << std::endl;
            // if (http_msg_end != m_buff_pos) {
            if (http_msg_end != std::end(m_buffer)) {
                auto http_msg = http_req_t::parse(&m_buffer.front(),
                                    std::distance(std::begin(m_buffer), http_msg_end-MSG_SEPARATOR.size()));
                                    // std::distance(std::begin(m_buffer), m_buff_pos-MSG_SEPARATOR.size()));

                if (!http_msg)
                    return false;

                if (http_msg->get_header("content-length")) {
                    BOOST_LOG_TRIVIAL(info) << "messages with body (e.g. with content-length) currently not supported!";
                    return false;
                }

                m_new_msg_cb(http_msg);

                std::vector<char> tmp_buf;
                tmp_buf.reserve(N);
                m_buff_pos = std::copy(http_msg_end + MSG_SEPARATOR.size(), m_buff_pos, std::begin(tmp_buf));
                m_buffer.swap(tmp_buf);
            }
        } while(http_msg_end != m_buff_pos); //check if multiple messages arrived in one packet/data bulk
        return true;
    }

    private:
    std::vector<char> m_buffer;
    std::vector<char>::iterator m_buff_pos;
    http_msg_reasembled_cb_t m_new_msg_cb;
};

