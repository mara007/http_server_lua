#include "server.h"
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <memory>

constexpr int PORT = 20000;
constexpr int THREADS = 3;

int main(int argc, char* args[]) {
    std::cout << "=== starting ===\n";
    try {
        server_t server(THREADS);

        server.register_new_conn_cb([](auto conn) {
            BOOST_LOG_TRIVIAL(info) << "=========== NEW CONN ===============";
            conn->register_new_msg_cb([](auto conn, auto http_req) {
                BOOST_LOG_TRIVIAL(info) << "=========== NEW REQ RECEIVED ===========\n" << *http_req;

                http_resp_t resp(200, "OK");
                resp.body = "<h1>HELLO WORLD OF HTTP!</h1>";
                conn->send_response(resp);
            });
        });

        std::cout << "=== listen on " << PORT << " ===\n";
        server.start_server(PORT);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
