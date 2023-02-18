#pragma once

#include <map>
#include <string>
#include <functional>
#include <algorithm>
#include <memory>
#include <optional>
#include <mutex>
#include <ostream>

#include <boost/log/trivial.hpp>

static const std::string MSG_SEPARATOR = "\r\n\r\n";
static const std::string NEW_LINE = "\r\n";

struct http_msg_with_headers_t {
    std::multimap<std::string, std::string> headers;

    //! if multiple headers 'name' present, returns the first one
    std::optional<std::string> get_header(const std::string& name);
    void add_header(std::string name, std::string value);
};

struct http_req_t : public http_msg_with_headers_t {
    std::string method;
    std::string path;
    std::string body;

    /*! parses give http message without the body - 'CLRF [ message-body ]' part 
     *  - header names will be in lowercase
     *  ABNF:
     *  - HTTP-message = start-line CRLF *( field-line CRLF ) CRLF [ message-body ]
     */
    static std::shared_ptr<http_req_t> parse(const char* data, size_t lenght);
};

struct http_resp_t : public http_msg_with_headers_t {
    std::string reason;
    std::string body;
    int code;

    http_resp_t(int c, std::string reas)
    : http_msg_with_headers_t(), reason(reas), body(), code(c)
    {}

    http_resp_t()
    : http_msg_with_headers_t(), reason(), body(), code()
    {}

    /*! serialized filled data to string. when body provided, 'content-lenght' header will be supplied automatically
     *  ABNF:
     *  - HTTP-message = status-line CRLF *( field-line CRLF ) CRLF [ message-body ]
     *  - status-line = HTTP-version SP status-code SP [ reason-phrase ]

     */
    std::string serialize_to_string() const;
};

std::ostream& operator<<(std::ostream& ostr, const http_msg_with_headers_t& msg_with_headers);
std::ostream& operator<<(std::ostream& ostr, const http_req_t& http_req);
std::ostream& operator<<(std::ostream& ostr, const http_resp_t& http_resp);

template<size_t N>
class http_buffer_t {
    public:
    using http_msg_reasembled_cb_t = std::function<void(std::shared_ptr<http_req_t>)>;

    http_buffer_t()
    : m_buffer(), m_msg_reassembled_cb(nullptr)
    {
        m_buffer.reserve(N);
    }

    http_buffer_t(http_msg_reasembled_cb_t cb)
    : m_buffer(), m_msg_reassembled_cb(std::move(cb))
    {
        m_buffer.reserve(N);
    }

    void register_msg_reasembled_cb(http_msg_reasembled_cb_t cb) {
        m_msg_reassembled_cb = cb;
    }

    bool new_data(const char* data, size_t length) {
        BOOST_LOG_TRIVIAL(debug) << "http_buffer_t::new_data() buffer.size=" << m_buffer.size()
                                 << ", buffer.capacity=" << m_buffer.capacity() << ", data length=" << length;
        if (m_buffer.size() + length > m_buffer.capacity())
            return false;

        auto guard = std::lock_guard(m_buff_mutex);
        std::copy(data, data+length, std::back_inserter(m_buffer));

        size_t http_msg_end;
        do {
            http_msg_end = m_buffer.find(MSG_SEPARATOR);

            if (http_msg_end != std::string::npos) {
                auto http_msg = http_req_t::parse(m_buffer.data(), http_msg_end);

                if (!http_msg)
                    return false;

                if (http_msg->get_header("content-length")) {
                    BOOST_LOG_TRIVIAL(info) << "messages with body (e.g. with content-length) currently not supported!";
                    return false;
                }

                m_buffer.erase(0, http_msg_end + MSG_SEPARATOR.size());
                if (m_msg_reassembled_cb) {
                    BOOST_LOG_TRIVIAL(info) << "reasebled callback: " << *http_msg;
                    m_msg_reassembled_cb(http_msg);
                }
            }
        } while(http_msg_end != std::string::npos); //check if multiple messages arrived in one packet/data bulk

        return true;
    }

    private:
    std::string m_buffer;
    std::mutex m_buff_mutex;
    http_msg_reasembled_cb_t m_msg_reassembled_cb;
};

