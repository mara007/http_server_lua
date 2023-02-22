#pragma once

#include <map>
#include <string>
#include <string_view>
#include <memory>
#include <optional>
#include <ostream>

#include <boost/log/trivial.hpp>

static const std::string MSG_SEPARATOR = "\r\n\r\n";
static const std::string NEW_LINE = "\r\n";
static const size_t MAX_BODY_SIZE = 1024;

struct http_msg_with_headers_t {
    std::multimap<std::string, std::string> headers;

    //! if multiple headers 'name' present, returns the first one
    std::optional<std::string> get_header(const std::string& name);
    void add_header(std::string name, std::string value);

    virtual ~http_msg_with_headers_t() = default;
};

struct http_req_t : public http_msg_with_headers_t {
    std::string method;
    std::string path;
    std::string body;
    std::multimap<std::string, std::string> params;

    /*! parses give http message without the body - 'CLRF [ message-body ]' part 
     *  - header names will be in lowercase
     *  ABNF:
     *  - HTTP-message = start-line CRLF *( field-line CRLF ) CRLF [ message-body ]
     */
    static std::shared_ptr<http_req_t> parse(const char* data, size_t lenght);

    /*! check if 'content-type' is 'application/x-www-form-urlencoded' and
     * parses form params from body if it is so.
     * TODO: decode special characters, e.g. %20 'space', %25 '&' etc.
     */
    void parse_body();

    virtual ~http_req_t() = default;
};

struct http_resp_t : public http_msg_with_headers_t {
    std::string reason;
    std::string body;
    int code;

    http_resp_t(int c, std::string reas)
    : http_msg_with_headers_t(), reason(reas), body(), code(c)
    {}

    http_resp_t() = default;
    http_resp_t(const http_resp_t&) = default;
    virtual ~http_resp_t() = default;

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

