#include "http/http_message.h"

#include <iostream>

#include "gtest/gtest.h"


TEST(http_req_t, parse) {
    std::string raw_msg = "GET /path/to/resource HTTP/1.1\r\n\
Host: example.com\r\n\
User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/109.0\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n\
Accept-Language: en-US,en;q=0.5\r\n\
Accept-Encoding: gzip, deflate\r\n\
Connection: keep-alive\r\n\
Upgrade-Insecure-Requests: 1\r\n\
Pragma: no-cache\r\n\
Cache-Control: no-cache\r\n\
";

    std::cout << "msg:" << std::endl << raw_msg << std::endl;

    auto parsed_msg = http_req_t::parse(raw_msg.c_str(), raw_msg.size());
    ASSERT_TRUE(parsed_msg.operator bool());
    ASSERT_EQ(parsed_msg->method, "get");
    ASSERT_EQ(parsed_msg->path, "/path/to/resource");
    ASSERT_EQ(parsed_msg->headers.size(), 9);

    ASSERT_EQ(parsed_msg->get_header("accept-encoding"), "gzip, deflate");
    ASSERT_EQ(parsed_msg->get_header("cache-control"), "no-cache");

    ASSERT_FALSE(parsed_msg->get_header("unknown-header"));

    parsed_msg->add_header("unknown-header", "my_value");
    ASSERT_EQ(parsed_msg->get_header("unknown-header"), "my_value");
}

TEST(http_req_t, parse_body_url_encoded_and_form_params) {
    std::string raw_msg = "GET /path/to/resource?qp1=qv1&qp2=qv2 HTTP/1.1\r\n\
Host: example.com\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Content-Length: 15\r\n\
";

// \
    std::cout << "msg:" << std::endl << raw_msg << std::endl;

    auto parsed_msg = http_req_t::parse(raw_msg.c_str(), raw_msg.size());
    parsed_msg->body = "fp1=fv1&fp2=fv2";
    parsed_msg->parse_body();

    ASSERT_TRUE(parsed_msg.operator bool());
    ASSERT_EQ(parsed_msg->get_param("qp1"), "qv1");
    ASSERT_EQ(parsed_msg->get_param("qp2"), "qv2");
    ASSERT_EQ(parsed_msg->get_param("fp1"), "fv1");
    ASSERT_EQ(parsed_msg->get_param("fp2"), "fv2");
}