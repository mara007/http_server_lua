#include "server/server.h"
#include "http/http_connection.h"
#include "lua/lua_manager.h"
#include "common/utils.h"

#include <iostream>
#include <memory>


int main(int argc, const char* argv[]) {
    auto cmd_line = cmd_line_t::parse(argc, argv);
    if (!cmd_line) {
        return 1;
    }

    std::cout << "=== starting ===\n"
              << "================\n"
              << "=== script : " << cmd_line.script << std::endl
              << "=== port   : " << cmd_line.port << std::endl
              << "=== threads: " << cmd_line.threads << std::endl
              << "================\n";

    auto lua_manager = std::make_shared<lua_manager_t>();
    if (!lua_manager->init(cmd_line.script, cmd_line.threads)) {
        return 2;
    }

    server_t<http_connection_t> server(cmd_line.threads);

    server.register_new_conn_cb([lua_manager](auto conn) {
        conn->register_new_msg_cb([lua_manager](auto conn, auto http_req) {
            http_resp_t http_resp(200, "OK");
            lua_manager->invoke_script(LUA_ENTRY_FUNCTION, http_req.get(), &http_resp);
            conn->send_response(http_resp);
        });
    });

    std::cout << "=== listen on " << cmd_line.port << " ===\n";

    try {
        server.start_server(cmd_line.port);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
