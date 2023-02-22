#include "http/http_buffer.h"

#include <iostream>
#include <memory>

#include "gtest/gtest.h"

using my_http_buffer_t = http_buffer_t<1024>;

TEST(http_buffer_t, msg_assemble_one) {

    std::shared_ptr<http_req_t> msg;
    size_t cb_called_times{0};
    auto cb = [&msg, &cb_called_times](decltype(msg) m) {
        msg = m;
        ++cb_called_times;
        std::cerr << "new msg cb fired.." << std::endl;
    };
    my_http_buffer_t http_buf(cb);

    std::string fragment1 = "GET /path/to/resource HTTP/1.1\r\n\
Host: example.com\r\n\
Conn";

    std::string fragment2 = "ection: keep-alive\r\n\
Upgrade-Insecure-Requests: 1\r\n\
Pragma: no-cache\r\n";

    std::string fragment3 = "Cache-Control: no-cache\r\n\r\n";

    std::cout << "..first fragment\n";
    ASSERT_EQ(http_buf.new_data(fragment1.c_str(), fragment1.size()), my_http_buffer_t::new_data_state_e::NEED_MORE_DATA);
    ASSERT_EQ(cb_called_times, 0);

    std::cout << "..second fragment\n";
    ASSERT_EQ(http_buf.new_data(fragment2.c_str(), fragment2.size()), my_http_buffer_t::new_data_state_e::NEED_MORE_DATA);
    ASSERT_EQ(cb_called_times, 0);

    std::cout << "..final fragment\n";
    ASSERT_EQ(http_buf.new_data(fragment3.c_str(), fragment3.size()), my_http_buffer_t::new_data_state_e::MSG_OK);
    ASSERT_EQ(cb_called_times, 1);
    ASSERT_EQ(msg->method, "get");
    ASSERT_EQ(msg->headers.size(), 5);
    ASSERT_EQ(msg->get_header("connection"), "keep-alive");
}

TEST(http_buffer_t, msg_assemble_two_in_one_fragment) {

    std::shared_ptr<http_req_t> msg;
    size_t cb_called_times{0};
    auto cb = [&msg, &cb_called_times](decltype(msg) m) {
        msg = m;
        ++cb_called_times;
        std::cerr << "new msg cb fired.." << std::endl;
    };
    http_buffer_t<1024> http_buf(cb);

    std::string fragment1 = "GET /path/to/resource HTTP/1.1\r\n\
Host: example.com\r\n\
Conn";

    std::string fragment2 = "ection: keep-alive\r\n\
Upgrade-Insecure-Requests: 1\r\n\
Pragma: no-cache\r\n";

    std::string fragment3 = "Cache-Control: no-cache\r\n\r\n\
GET /this/is/another/msg HTTP/1.1\r\n\
Host: sample.com\r\n\r\n";

    std::cout << "..first fragment\n";
    ASSERT_EQ(http_buf.new_data(fragment1.c_str(), fragment1.size()), my_http_buffer_t::new_data_state_e::NEED_MORE_DATA);
    ASSERT_EQ(cb_called_times, 0);

    std::cout << "..second fragment\n";
    ASSERT_EQ(http_buf.new_data(fragment2.c_str(), fragment2.size()), my_http_buffer_t::new_data_state_e::NEED_MORE_DATA);
    ASSERT_EQ(cb_called_times, 0);

    std::cout << "..final fragment\n";
    ASSERT_EQ(http_buf.new_data(fragment3.c_str(), fragment3.size()), my_http_buffer_t::new_data_state_e::MSG_OK);
    ASSERT_EQ(cb_called_times, 2);
    ASSERT_EQ(msg->method, "get");
    ASSERT_EQ(msg->headers.size(), 1);
    ASSERT_EQ(msg->get_header("host"), "sample.com");
}

