#include "http/http_message.h"


#include "lua.hpp"
#include <boost/log/trivial.hpp>


static int dummy(lua_State* l) {
    return 0;
}

int http_req_new(lua_State* l){

}

/*************************************************/
/****************** MSG COMMON *******************/
/*************************************************/

int http_msg_add_header(lua_State* l, const char* meta_table) {
    int argc = lua_gettop(l);
    if (argc != 3) {
        return luaL_error(l, "http[req/resp] - invalid number of arguments");
    }

    http_msg_with_headers_t* http_msg = (http_msg_with_headers_t *)luaL_checkudata(l, 1, meta_table);
    luaL_argcheck(l, http_msg!= nullptr, 1, "http[req/resp] msg object expected");

    const char* header_name = lua_tolstring(l, 2, nullptr);
    const char* header_value = lua_tolstring(l, 3, nullptr);

    if (!header_name || !header_value) {
        return luaL_error(l, "http_req/resp:add_header() got NIL!");
    }

    BOOST_LOG_TRIVIAL(debug) << "lua: http_msg:add_header('" << header_name << "', '" << header_value << "') ptr=" << (void*)http_msg;
    http_msg->add_header(header_name, header_value);
    return 0;
}

int http_msg_get_header(lua_State* l, const char* meta_table) {
    int argc = lua_gettop(l);
    if (argc != 2) {
        return luaL_error(l, "http[req/resp] - invalid number of arguments");
    }

    http_msg_with_headers_t* http_msg = (http_msg_with_headers_t *)luaL_checkudata(l, 1, meta_table);
    luaL_argcheck(l, http_msg!= nullptr, 1, "http[req/resp] msg object expected");
    auto header_name = lua_tolstring(l, 2, nullptr);
    if (!header_name ) {
        return luaL_error(l, "http_req/resp:add_header() got NIL!");
    }

    BOOST_LOG_TRIVIAL(debug) << "lua: http_msg:get_header('" << header_name << "')";
    if (auto header_val = http_msg->get_header(header_name); header_val) {
        lua_pushlstring(l, header_val.value().c_str(), header_val.value().size());
        return 1;
    }

    lua_pushnil(l);
    return 1;


}
/*************************************************/
/****************** RESPONSE *********************/
/*************************************************/
int http_resp_new(lua_State* l){
    http_resp_t *m = new http_resp_t();
    BOOST_LOG_TRIVIAL(debug) << "lua: http_resp:new() ptr=" << (void*)m;
    lua_pushlightuserdata(l, (void*)m);
    luaL_setmetatable(l, "http_resp.meta");
    return 1;
}

int http_resp_del(lua_State* l){
    if (!lua_islightuserdata(l, 1)) {
        return luaL_error(l, "expected http_resp object");
    }
    http_resp_t *m = (http_resp_t*)lua_touserdata(l, 1);

    BOOST_LOG_TRIVIAL(debug) << "lua: http_resp:~() ptr=" << (void*)m;

    delete m;
    return 0;
}


int http_resp_add_header(lua_State* l) {
    return http_msg_add_header(l, "http_resp.meta");
}

int http_resp_get_header(lua_State* l) {
    return http_msg_get_header(l, "http_resp.meta");
}

void register_http_response(lua_State* l) {
    BOOST_LOG_TRIVIAL(debug) << "lua: register http_resp";
    static const luaL_Reg http_resp_meta[] = {
        {"new", http_resp_new},
        {"delete", http_resp_del},
        {"add_header", http_resp_add_header},
        {"get_header", http_resp_get_header},
        { nullptr, nullptr }
    };

    luaL_newmetatable(l, "http_resp.meta");
    luaL_setfuncs(l, http_resp_meta, 0);
    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    luaL_newlib(l, http_resp_meta);
    lua_setglobal(l, "http_resp");
}

void register_custom_functions(lua_State* l) {

    register_http_response(l);

}
