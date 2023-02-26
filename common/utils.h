#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <algorithm>


std::vector<std::string_view> tokenize(std::string_view data, std::string_view separator, bool consume_empty=false);

std::string to_lowercase(std::string_view s);
bool to_int(const std::string& s, int& result);

struct cmd_line_t {
    bool is_ok;
    std::string script;
    unsigned port;
    unsigned threads;

    static cmd_line_t parse(int argc, const char* argv[]);
    operator bool();

    private:
    cmd_line_t();
    static void print_help();
};