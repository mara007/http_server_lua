#include "server.h"
#include <boost/asio.hpp>
#include <iostream>

constexpr int PORT = 20000;

int main(int argc, char* args[]) {
    std::cout << "=== starting ===\n";
    try {
        boost::asio::io_context io_context;
        server_t server(io_context, PORT);
        std::cout << "=== listen on " << PORT << " ===\n";
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
