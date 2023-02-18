#include "http.hpp"

#include <iostream>
#include <memory>

#include "gtest/gtest.h"

TEST(http_buffer_t, msg_assemble) {

    std::shared_ptr<message_t> msg;
    size_t cb_called_times{0};
    auto cb = [&msg, &cb_called_times](decltype(msg) m) {
        msg = m;
        ++cb_called_times;
    };
    http_buffer_t<1024> http_buf(cb);

    std::string fragment1 = "GET /path/to/resource HTTP/1.1\r\n\
Host: example.com\r\n\
Conn";

    std::string fragment2 = "ection: keep-alive\r\n\
Upgrade-Insecure-Requests: 1\r\n\
Pragma: no-cache\r\n";

    std::string fragment3 = "Cache-Control: no-cache\r\n\r\n";

    std::cout << "..first fragment\n";
    ASSERT_TRUE(http_buf.new_data(fragment1.c_str(), fragment1.size()));
    ASSERT_EQ(cb_called_times, 0);

    std::cout << "..second fragment\n";
    ASSERT_TRUE(http_buf.new_data(fragment2.c_str(), fragment2.size()));
    ASSERT_EQ(cb_called_times, 0);

    std::cout << "..final fragment\n";
    ASSERT_TRUE(http_buf.new_data(fragment3.c_str(), fragment3.size()));
    ASSERT_EQ(cb_called_times, 1);
    ASSERT_EQ(msg->method, "get");
    ASSERT_EQ(msg->headers.size(), 5);
    ASSERT_EQ(msg->get_header("connection"), "keep-alive");

}