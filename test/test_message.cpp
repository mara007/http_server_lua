#include "http.hpp"

#include <iostream>

#include "gtest/gtest.h"

std::vector<std::string_view> tokenize(std::string_view tokenize_this, std::string_view by_this, bool consume_empty);

TEST(tokenize, empty) {
    std::string empty;
    auto res = tokenize(empty, "", false);

    ASSERT_TRUE(res.empty());
}

TEST(tokenize, one) {
    std::string data{"one"};
    auto res = tokenize(data, " ", false);

    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ(res[0], "one");
}

TEST(tokenize, two) {
    std::string data{"one two"};
    auto res = tokenize(data, " ", false);

    ASSERT_EQ(res.size(), 2);
    ASSERT_EQ(res[0], "one");
    ASSERT_EQ(res[1], "two");
}

TEST(tokenize, two_extra_separators) {
    std::string data{"one   two"};
    auto res = tokenize(data, " ", false);

    ASSERT_EQ(res.size(), 4);
    ASSERT_EQ(res[0], "one");
    ASSERT_EQ(res[1], "");
    ASSERT_EQ(res[2], "");
    ASSERT_EQ(res[3], "two");
}

TEST(tokenize, two_extra_separators_consume) {
    std::string data{"one   two"};
    auto res = tokenize(data, " ", true);

    ASSERT_EQ(res.size(), 2);
    ASSERT_EQ(res[0], "one");
    ASSERT_EQ(res[1], "two");
}

TEST(tokenize, three) {
    std::string data{"one two three"};
    auto res = tokenize(data, " ", false);

    ASSERT_EQ(res.size(), 3);
    ASSERT_EQ(res[0], "one");
    ASSERT_EQ(res[1], "two");
    ASSERT_EQ(res[2], "three");
}

TEST(message_t, parse) {
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

    auto parsed_msg = message_t::parse(raw_msg.c_str(), raw_msg.size());
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