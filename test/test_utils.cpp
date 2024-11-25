#include "common/timer.h"
#include "common/utils.h"

#include <iostream>

#include "gtest/gtest.h"
#include <chrono>
#include <thread>


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

// TEST(tokenize, crash) {
//     std::string data{"HTTP/1.1 get /\r\nHTTP/1.1 get /\r\nHTTP/1.1 get /\r\n\r\n"};
//     auto res = tokenize(data, "\r\n", false);
//
//     ASSERT_EQ(res.size(), 4);
//
//     auto res2 = tokenize(res.front(), " ", true);
//     std::cout << res2[0] << res2[1] << res2[2] << std::endl;
//     std::cout << res2[3];
//     ASSERT_EQ(res2[0], "one");
//     ASSERT_EQ(res2[1], "two");
//     ASSERT_EQ(res2[2], "three");
// }
//

// *********************** CMDLINE ****************************

TEST(cmd_line_t, defaults) {
    const char * cmd_line[] = {"bin_name"};
    auto cmd = cmd_line_t::parse(1, cmd_line);

    ASSERT_TRUE(cmd.operator bool());
    ASSERT_EQ(cmd.port, 20000);
    ASSERT_EQ(cmd.threads, 3);
    ASSERT_EQ(cmd.script, "www/http_server.lua");
}

TEST(cmd_line_t, all_params) {
    const char* cmd_line[] = {"bin_name", "--script", "my_script.lua", "-p", "1234", "--threads", "22"};
    auto cmd = cmd_line_t::parse(7, cmd_line);

    ASSERT_TRUE(cmd.operator bool());
    ASSERT_EQ(cmd.port, 1234);
    ASSERT_EQ(cmd.threads, 22);
    ASSERT_EQ(cmd.script, "my_script.lua");
}

TEST(cmd_line_t, some_param) {
    const char* cmd_line[] = {"bin_name", "--script", "my_script.lua"};
    auto cmd = cmd_line_t::parse(3, cmd_line);

    ASSERT_TRUE(cmd.operator bool());
    ASSERT_EQ(cmd.port, 20000);
    ASSERT_EQ(cmd.threads, 3);
    ASSERT_EQ(cmd.script, "my_script.lua");
}

// ********************** TIMER *****************************
//
using namespace std::chrono_literals;

TEST(my_timer_t, start_stop) {
    my_timer_t timer(500ms);
    timer.init();
    timer.finish();
}

TEST(my_timer_t, timer_expires) {
    my_timer_t timer(500ms);
    timer.init();

    int invoke_count = 0;
    auto my_cb = [&](std::string id){
        return [&, id] {
            ++invoke_count;
            std::cout << "cb called: " << id << " order: " << invoke_count << std::endl;
        };
    };

    timer.add(my_cb("3s"), 3s);
    timer.add(my_cb("2s - a"), 2s);
    timer.add(my_cb("2s - b"), 2s);
    timer.add(my_cb("2s - c"), 2s);
    timer.add(my_cb("2s - d"), 2s);
    timer.add(my_cb("1s"), 1s);

    std::this_thread::sleep_for(4s);
    ASSERT_EQ(invoke_count, 6);

    timer.finish();
}





