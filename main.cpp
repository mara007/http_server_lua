#include "server.h"
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <memory>

constexpr int PORT = 20000;

int main(int argc, char* args[]) {
    std::cout << "=== starting ===\n";
    try {
        boost::asio::io_context io_context;
        server_t server(io_context, PORT);
        server.register_new_conn_cb([](auto conn) {
            BOOST_LOG_TRIVIAL(info) << "=========== NEW CONN ===============";
            conn->register_new_msg_cb([](auto conn, auto http_req) {
                BOOST_LOG_TRIVIAL(info) << "=========== NEW REQ RECEIVED ===========\n" << *http_req;
            });
        });
        std::cout << "=== listen on " << PORT << " ===\n";
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
