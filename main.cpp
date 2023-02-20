#include "server/server.h"
#include "http/http_connection.h"
#include "lua/lua_manager.h"

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

#include <iostream>
#include <memory>

constexpr int PORT = 20000;
constexpr int THREADS = 4;

int main(int argc, char* args[]) {
    std::cout << "=== starting ===\n";

    auto lua_manager = std::make_shared<lua_manager_t>();
    if (!lua_manager->init("http_server.lua", THREADS)) {
        return 1;
    }

    server_t<http_connection_t> server(THREADS);

    server.register_new_conn_cb([lua_manager](auto conn) {
        BOOST_LOG_TRIVIAL(info) << "=========== NEW CONN ===============";
        conn->register_new_msg_cb([lua_manager](auto conn, auto http_req) {
            BOOST_LOG_TRIVIAL(info) << "=========== NEW REQ RECEIVED ===========\n" << *http_req;

            http_resp_t http_resp(200, "OK");
            lua_manager->invoke_script(*http_req, http_resp);
            http_resp.body.append("<p>");
            http_resp.body.append(http_req->path);
            http_resp.body.append("</p>");
            BOOST_LOG_TRIVIAL(info) << "MAIN send response";
            conn->send_response(http_resp);
            // conn->close();
        });
    });

    std::cout << "=== listen on " << PORT << " ===\n";

    try {
        server.start_server(PORT);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
