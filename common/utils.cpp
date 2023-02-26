#include "utils.h"

#include <boost/log/trivial.hpp>

#include <iostream>
#include <string>
#include <stdexcept>

static const std::string DEFAULT_SCRIPT = "www/http_server.lua";
static unsigned DEFAULT_PORT = 20000;
static unsigned DEFAULT_THREADS = 3;

std::vector<std::string_view> tokenize(std::string_view data, std::string_view separator, bool consume_empty) {
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

bool to_int(const std::string& s, int& result) {
    try {
        result = std::stoi(s);
    } catch (std::invalid_argument const& e) {
        return false;
    } catch (std::out_of_range const& e) {
        return false;
    }

    return true;
}

// ************** cmd_line_t **************

cmd_line_t::cmd_line_t()
: is_ok(false)
, script(DEFAULT_SCRIPT)
, port(DEFAULT_PORT)
, threads(DEFAULT_THREADS)
{}

cmd_line_t::operator bool() {
    return is_ok;
}


cmd_line_t cmd_line_t::parse(int argc, const char* argv[]) {
    enum state_e : uint8_t {
        HELP = 0,
        SCRIPT,
        PORT,
        THREADS,
        NONE,
        ERROR,
    };

    cmd_line_t result;
    result.is_ok = true;

    if (argc < 1)
        return result;

    state_e state = state_e::NONE;

        std::cerr << "argc: '" << argc << "'\n";
    int int_arg;
    for (int i = 1; i < argc; ++i) {
        std::string arg{argv[i]};
        std::cerr << "i=" << i << ", ARG: '" << arg << "' state: " << (int)state << "\n";

        if (arg == "-h" || arg == "--help" ) {
            state = state_e::HELP;
        } else
        if (arg == "-p" || arg == "--port") {
            state = state_e::PORT;
                continue;
        } else
        if (arg == "-s" || arg == "--script") {
            state = state_e::SCRIPT;
                continue;
        } else
        if (arg == "-t" || arg == "--threads") {
            state = state_e::THREADS;
                continue;
        } else {
            if (state == state_e::NONE) {
                std::cerr << "unknown param: " << arg << std::endl;
                result.is_ok = false;
                print_help();
                return result;
            }
        }

        switch (state) {
            case state_e::HELP:
                result.is_ok = false;
                print_help();
                return result;

            case state_e::SCRIPT:
                result.script = arg;
                state = state_e::NONE;
                continue;

            case state_e::PORT:
                if (to_int(arg, int_arg); int_arg > 0) {
                    result.port = static_cast<unsigned>(int_arg);
                    state = state_e::NONE;
                    continue;
                }
                std::cerr << "wrong arg: port must be a possitive integer" << std::endl;
                result.is_ok = false;
                print_help();
                return result;

            case state_e::THREADS:
                if (to_int(arg, int_arg); int_arg > 0) {
                    result.threads = static_cast<unsigned>(int_arg);
                    state = state_e::NONE;
                    continue;
                }
                std::cerr << "wrong arg: threads must be a possitive integer" << std::endl;
                result.is_ok = false;
                print_help();
                return result;

            case state_e::NONE:
                if (i) {
                    result.is_ok = false;
                    print_help();
                    return result;
                }
            default: ;
        }

        std::cerr << "FIRSROUND FALSE\n";
    }

    return result;
}

void cmd_line_t::print_help() {
    std::cout << "Usage: http_server_lua [OPTION]..." << std::endl
              << "starts a http server" << std::endl << std::endl
              << "Arguments are:" << std::endl
              << "-s, --script       lua script to process incomming traffic" << std::endl
              << "                   default: " << DEFAULT_SCRIPT << std::endl
              << "-p, --port         a port on which server will listen on all ipv4 interfaces" << std::endl
              << "                   default: " << DEFAULT_PORT << std::endl
              << "-t, --threads      number of worker threads to use" << std::endl
              << "                   default: " << DEFAULT_THREADS
              << std::endl;
}