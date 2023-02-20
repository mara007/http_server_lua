#include "http/http_message.h"


#include "lua.hpp"
#include <boost/log/trivial.hpp>


static int dummy(lua_State* l) {
    return 0;
}

int http_req_new(lua_State* l){

}

int http_resp_new(lua_State* l){
    http_resp_t *m = new http_resp_t();
    BOOST_LOG_TRIVIAL(debug) << "lua: http_resp:new() ptr=" << (void*)m;
    lua_pushlightuserdata(l, (void*)m);
    luaL_setmetatable(l, "http_resp__");
    return 1;
}

int http_msg_add_header(lua_State *l, const char* meta_table) {
    int argc = lua_gettop(l);
    if (argc != 3) {
        return luaL_error(l, "http[req/resp] - invalid number of arguments");
    }

    http_msg_with_headers_t **http_msg_data  = (http_msg_with_headers_t **)luaL_checkudata(l, 1, "http_resp__");
    luaL_argcheck(l, http_msg_data != nullptr, 1, "http[req/resp] msg object expected");
    http_msg_with_headers_t *http_msg = *http_msg_data;

    const char *header_name = lua_tolstring(l, 2, nullptr);
    const char *header_value = lua_tolstring(l, 3, nullptr);

    BOOST_LOG_TRIVIAL(debug) << "lua: http_msg:add_header('" << header_name << "', '" << header_value << "') ptr=" << (void*)http_msg;
    http_msg->add_header(header_name, header_value);
    return 0;
}

int http_resp_add_header(lua_State *l) {
    return http_msg_add_header(l, "http_resp__");
}

void register_custom_functions(lua_State *l) {

    static const luaL_Reg __meta_table[] = {
        {"__gc", dummy},
        {"__index", dummy},
        {"__newindex", dummy},
        { nullptr, nullptr }
    };

    static const luaL_Reg __metas[] = {
        {"__gc", dummy},
        {"__index", dummy},
        {"__newindex", dummy},
        { nullptr, nullptr }
    };

    static const luaL_Reg __http_resp_methods[] = {
        {"add_header", http_resp_add_header},
        {"new", http_resp_new},
        { nullptr, nullptr }
    };

    int lib_id, meta_id;

    lua_createtable(l, 0, 0);
    lib_id = lua_gettop(l);
    luaL_newmetatable(l, "http_resp__");
    meta_id = lua_gettop(l);
    luaL_setfuncs(l, __meta_table, 0);

    luaL_newlib(l, __http_resp_methods);
    lua_setfield(l, meta_id, "__index");

    luaL_newlib(l, __metas);
    lua_setfield(l, meta_id, "__metatable");

    lua_setmetatable(l, lib_id);
    lua_setglobal(l, "http_response");
}
