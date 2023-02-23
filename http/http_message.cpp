#include "http_message.h"

#include <string>
#include <vector>
#include <string_view>
#include <algorithm>

#include <boost/log/trivial.hpp>

static const std::string HTTP_VERS = "HTTP/1.1";
static const std::string X_WWW_FORM_ENCODED = "application/x-www-form-urlencoded";
static const std::string PARAMS_SEPARATOR = "&";
static const std::string QUERY_PARAMS_SEPARATOR = "?";
static const std::string VALUE_SEPARATOR = "=";

std::vector<std::string_view> tokenize(std::string_view data, std::string_view separator, bool consume_empty=false) {
    if (separator.empty())
        return {};

    size_t last_pos = 0;
    auto pos = data.find(separator);

    std::vector<std::string_view> result;
    while (pos != std::string::npos) {
        auto token = data.substr(last_pos, pos - last_pos);
        // BOOST_LOG_TRIVIAL(debug) << "token: " << token;

        if (not (consume_empty && token.empty()))
            result.push_back(token);

        last_pos = pos + separator.size();
        pos = data.find(separator, last_pos);
    }

    if (last_pos == data.size()) //skip last separator
        return result;

    auto token = data.substr(last_pos);
    // BOOST_LOG_TRIVIAL(debug) << "token: " << token;

    if (not (consume_empty && token.empty()))
        result.push_back(token);

    return result;
}

std::string to_lowercase(std::string_view s) {
    std::string result;
    std::transform(std::begin(s), std::end(s),
        std::back_inserter(result),
        [](auto c) {
            return std::tolower(c);
        }
    );

    return result;
}

void parse_url_encoded_params(std::string_view encoded_params, std::multimap<std::string, std::string>& result) {
    for (auto param : tokenize(encoded_params, PARAMS_SEPARATOR)) {
        auto pos = param.find(VALUE_SEPARATOR);
        if(pos == std::string::npos)
            continue;
        auto param_name = param.substr(0, pos);
        auto param_val  = param.substr(pos+1);
        BOOST_LOG_TRIVIAL(debug) << "new param - '" << param_name << "' : '" << param_val << "'";

        result.emplace(param_name, param_val);
    }
}

std::shared_ptr<http_req_t> http_req_t::parse(const char* data, size_t lenght) {
    /* TODO
      HTTP 1/1 - mandatory header : host * 1
    */
    std::string_view raw_msg(data, lenght);
    std::string_view NL(NEW_LINE);
    std::vector<std::string_view> lines = tokenize(raw_msg, NL, false);

    if (lines.size() < 2) {
        BOOST_LOG_TRIVIAL(info) << "invalid - less then 2 lines";
        return nullptr;
    }

    auto start_line = tokenize(lines.front(), " ", true);
    if (start_line.size() != 3) {
        BOOST_LOG_TRIVIAL(info) << "invalid - request line";
        return nullptr;
    }

    if (start_line[2] != HTTP_VERS) {
        BOOST_LOG_TRIVIAL(info) << "invalid - bad http version: " << start_line[3];
        return nullptr;
    }
    auto msg = std::make_shared<http_req_t>();
    msg->method = to_lowercase(start_line[0]);
    msg->path = start_line[1];

    for (auto it = std::begin(lines)+1; it != std::end(lines); ++it) {
        auto colon_pos = it->find(":");
        if (colon_pos == std::string::npos) {
            BOOST_LOG_TRIVIAL(info) << "invalid - no colon in header: " << *it;
            return nullptr;
        }

        if (colon_pos == it->length()) {
            BOOST_LOG_TRIVIAL(info) << "invalid - colon as a last char - no value";
            return nullptr;
 
        }
        auto header_name = it->substr(0, colon_pos);
        auto header_val  = it->substr(colon_pos+1);
        if (header_val.front() == ' ')
            header_val.remove_prefix(1);

        if (header_val.back() == ' ')
            header_val.remove_suffix(1);

        std::string lowercase_header_name = to_lowercase(header_name);
        // BOOST_LOG_TRIVIAL(debug) << "new header - '" << lowercase_header_name << "' : '" << header_val << "'";
        msg->headers.emplace(std::move(lowercase_header_name), header_val);
    }

    // url encded params
    std::string_view path_view(msg->path);
    if (auto pos = path_view.find(QUERY_PARAMS_SEPARATOR); pos != path_view.size()-1 && pos != std::string::npos) {
        parse_url_encoded_params(path_view.substr(pos+1), msg->params);
        msg->path.erase(pos);
    }

    return msg;
}

void http_req_t::parse_body() {
    if (headers.empty()) {
        return;
    }

    auto cth = get_header("content-type");
    if (!cth)
        return;

    std::string content_type_lower = to_lowercase(cth.value());
    if (content_type_lower != X_WWW_FORM_ENCODED)
        return;

    parse_url_encoded_params(body, params);
}

std::optional<std::string> http_req_t::get_param(const std::string& name) {
    if (auto p = params.find(name); p != std::end(params))
        return p->second;

    return std::nullopt;
}

std::optional<std::string> http_msg_with_headers_t::get_header(const std::string& name) {
    if (auto h = headers.find(name); h != std::end(headers))
        return h->second;

    return std::nullopt;
}

void http_msg_with_headers_t::add_header(std::string name, std::string value) {
    headers.insert(std::pair{name, value});
}


std::ostream& operator<<(std::ostream& ostr, const http_msg_with_headers_t& msg_with_headers) {
    ostr << "HEADRS:";
    for (const auto& [k, v]: msg_with_headers.headers)
        ostr << "\t" << k << ": " << v << std::endl;
    if (msg_with_headers.headers.empty())
        ostr << std::endl;
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const http_req_t& http_req) {
    ostr << "HTTP REQ: " << HTTP_VERS << " " << http_req.method << " " << http_req.path << "\n";
    ostr << static_cast<http_msg_with_headers_t>(http_req);
    ostr << "PARAMS:";
    for (const auto& [k, v]: http_req.params)
        ostr << "\t" << k << ": " << v << std::endl;

    ostr << "BODY: " << http_req.body;

    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const http_resp_t& http_resp) {
    ostr << "HTTP RESP: " << http_resp.code << " " << http_resp.reason << "\n";
    ostr << static_cast<http_msg_with_headers_t>(http_resp);
    ostr << "BODY: " << http_resp.body;

    return ostr;
}


std::string http_resp_t::serialize_to_string() const {
    static const std::string SEP{": "};
    std::string result;
    result.reserve(64);

    // status-line = HTTP-version SP status-code SP [ reason-phrase ]
    result.append(HTTP_VERS);
    result.append(1, ' ');
    result.append(std::to_string(code));
    result.append(1, ' ');
    result.append(reason);
    result.append(NEW_LINE);

    // *( field-line CRLF )
    for (const auto& [k,v]: headers) {
        result.append(k);
        result.append(SEP);
        result.append(v);
        result.append(NEW_LINE);
    }

    if (!body.empty()) {
        result.append("content-length");
        result.append(SEP);
        result.append(std::to_string(body.length()));
        result.append(NEW_LINE);

        result.append(NEW_LINE);
        result.append(body);
    } else {
        result.append("content-length"); // prevent client's 'Chunked transfer encoding'
        result.append(SEP);
        result.append("0");
        result.append(NEW_LINE);
        result.append(NEW_LINE);
    }

    return result;
}