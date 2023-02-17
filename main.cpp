#include "server.h"
#include <boost/asio.hpp>
#include <iostream>

int main(int argc, char* args[]) {
    std::cout << "=== starting ===\n";
    try {
        boost::asio::io_context io_context;
        server_t server(io_context, 20000);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
