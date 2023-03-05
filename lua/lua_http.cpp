#include "lua_http.h"
#include "lua_shared_storage.h"

#include "http/http_message.h"

#include "lua.hpp"
#include <boost/log/trivial.hpp>

/*************************************************/
/****************** COMMON ***********************/
/*************************************************/

/*************************************************/
/****************** MSG COMMON *******************/
/*************************************************/

//! check for given meta_table type on top of the lua stack
template<typename T>
T* my_check_type(lua_State* l, int no_of_args, const char* meta_table) {
    if (lua_gettop(l) != no_of_args) {
        luaL_error(l, "invalid number of arguments");
        return nullptr;
    }

    auto **msg  = (T **)luaL_checkudata(l, 1, meta_table);
    luaL_argcheck(l, msg != nullptr, 1, "http req/resp object expected");
    return msg ? *msg : nullptr;
}

namespace http_msg_common
{
int add_header(lua_State* l, const char* meta_table) {
    auto http_msg = my_check_type<http_msg_with_headers_t>(l, 3, meta_table);

    const char* header_name = lua_tolstring(l, 2, nullptr);
    const char* header_value = lua_tolstring(l, 3, nullptr);

    if (!header_name || !header_value) {
        return luaL_error(l, "http_req/resp:add_header() got NIL!");
    }

    BOOST_LOG_TRIVIAL(debug) << "lua: http_msg:add_header('" << header_name << "', '" << header_value << "') ptr=" << (void*)http_msg;
    http_msg->add_header(header_name, header_value);
    return 0;
}

int get_header(lua_State* l, const char* meta_table) {
    auto http_msg = my_check_type<http_msg_with_headers_t>(l, 2, meta_table);
    if (!http_msg) return 1;

    auto header_name = lua_tolstring(l, 2, nullptr);
    if (!header_name ) {
        return luaL_error(l, "http_req/resp:add_header() got NIL!");
    }

    BOOST_LOG_TRIVIAL(debug) << "lua: http_msg:get_header('" << header_name << "') ptr=" << (void*)http_msg;
    if (auto header_val = http_msg->get_header(header_name); header_val) {
        lua_pushlstring(l, header_val.value().c_str(), header_val.value().size());
        return 1;
    }

    lua_pushnil(l);
    return 1;
}

} // namespace http_msg_common

/*************************************************/
/****************** REQUEST **********************/
/*************************************************/

namespace http_req
{
int new_msg(lua_State* l){
    http_req_t *m = new http_req_t();
    BOOST_LOG_TRIVIAL(debug) << "lua: http_req:new() ptr=" << (void*)m;
    lua_pushlightuserdata(l, (void*)m);
    luaL_setmetatable(l, LUA_HTTP_REQ_META);
    return 1;
}

int del_msg(lua_State* l){
    if (!lua_islightuserdata(l, 1)) {
        return luaL_error(l, "expected http_req object");
    }
    http_req_t *m = reinterpret_cast<http_req_t*>(lua_touserdata(l, 1));

    BOOST_LOG_TRIVIAL(debug) << "lua: http_req:~() ptr=" << (void*)m;

    delete m;
    return 0;
}

int add_header(lua_State* l) {
    return http_msg_common::add_header(l, LUA_HTTP_REQ_META);
}

int get_header(lua_State* l) {
    return http_msg_common::get_header(l, LUA_HTTP_REQ_META);
}

int get_method(lua_State* l){
    auto http_req = my_check_type<http_req_t>(l, 1, LUA_HTTP_REQ_META);
    if (!http_req) return 1;

    lua_pushlstring(l, http_req->method.data(), http_req->method.size());
    BOOST_LOG_TRIVIAL(debug) << "lua: http_req:get_method() method=" << http_req->method;
    return 1;
}

int get_path(lua_State* l){
    auto http_req = my_check_type<http_req_t>(l, 1, LUA_HTTP_REQ_META);
    if (!http_req) return 1;

    lua_pushlstring(l, http_req->path.data(), http_req->path.size());
    BOOST_LOG_TRIVIAL(debug) << "lua: http_req:get_path() path=" << http_req->path;
    return 1;
}

int get_body(lua_State* l){
    auto http_req = my_check_type<http_req_t>(l, 1, LUA_HTTP_REQ_META);
    if (!http_req) return 1;

    lua_pushlstring(l, http_req->body.data(), http_req->body.size());
    BOOST_LOG_TRIVIAL(debug) << "lua: http_req:get_body() size=" << http_req->body.size();
    return 1;
}

int get_param(lua_State* l){
    auto http_req = my_check_type<http_req_t>(l, 2, LUA_HTTP_REQ_META);
    if (!http_req) return 1;

    auto param_name = lua_tolstring(l, 2, nullptr);
    if (!param_name)
        return luaL_error(l, "parameter is not a string");

    if (auto param_val = http_req->get_param(param_name); param_val) {
        BOOST_LOG_TRIVIAL(debug) << "lua: http_req:get_param(" << param_name << ") value=" << param_val.value();
        lua_pushlstring(l, param_val.value().data(), param_val.value().size());
    } else {
        BOOST_LOG_TRIVIAL(debug) << "lua: http_req:get_param(" << param_name << ") value=NIL";
        lua_pushnil(l);
    }
    return 1;
}

int dump(lua_State* l) {
    auto http_req = my_check_type<http_req_t>(l, 1, LUA_HTTP_REQ_META);
    if (!http_req) return 0;

    BOOST_LOG_TRIVIAL(debug) << "lua: http_req:dump(): ptr=" << (void*)http_req << "\n" << *http_req;
    return 0;
}

void register_type(lua_State* l) {
    BOOST_LOG_TRIVIAL(debug) << "lua: register http_req";
    static const luaL_Reg http_req_meta[] = {
        {"new", new_msg},
        {"delete", del_msg},
        {"dump", dump},
        {"add_header", add_header},
        {"get_header", get_header},
        {"get_method", get_method},
        {"get_path", get_path},
        {"get_body", get_body},
        {"get_param", get_param},
        { nullptr, nullptr }
    };

    // create http_req
    lua_createtable(l, 0, 0);
    int lib_id = lua_gettop(l);
    luaL_newmetatable(l, LUA_HTTP_REQ_META);
    int meta_id = lua_gettop(l);
    luaL_setfuncs(l, http_req_meta, 0);

    luaL_newlib(l, http_req_meta);
    lua_setfield(l, meta_id, "__index");

    luaL_newlib(l,http_req_meta);
    lua_setfield(l, meta_id, "__metatable");

    lua_setmetatable(l, lib_id);
    lua_setglobal(l, "http_req");

}


} //namespace http_req

/*************************************************/
/****************** RESPONSE *********************/
/*************************************************/

namespace http_resp
{
int new_msg(lua_State* l){
    http_resp_t *m = new http_resp_t();
    BOOST_LOG_TRIVIAL(debug) << "lua: http_resp:new() ptr=" << (void*)m;
    lua_pushlightuserdata(l, (void*)m);
    luaL_setmetatable(l, LUA_HTTP_RESP_META);
    return 1;
}

int del_msg(lua_State* l){
    if (!lua_islightuserdata(l, 1)) {
        return luaL_error(l, "expected http_resp object");
    }
    http_resp_t *m = reinterpret_cast<http_resp_t*>(lua_touserdata(l, 1));

    BOOST_LOG_TRIVIAL(debug) << "lua: http_resp:~() ptr=" << (void*)m;

    delete m;
    return 0;
}

int add_header(lua_State* l) {
    return http_msg_common::add_header(l, LUA_HTTP_RESP_META);
}

int get_header(lua_State* l) {
    return http_msg_common::get_header(l, LUA_HTTP_RESP_META);
}

int set_status_code(lua_State* l){
    auto http_resp = my_check_type<http_resp_t>(l, 2, LUA_HTTP_RESP_META);
    if (!http_resp) return 1;


    int code = lua_tointeger(l, 2);
    if (code <= 0 || code >= 600) {
        return luaL_error(l, "http_resp:set_status_code() - http status code %d is invalid", code);
    }

    BOOST_LOG_TRIVIAL(debug) << "lua: http_resp:set_status_code(" << code << ")";
    http_resp->code = code;
    return 0;
}

int set_reason(lua_State* l){
    auto http_resp = my_check_type<http_resp_t>(l, 2, LUA_HTTP_RESP_META);
    if (!http_resp) return 1;

    const char* reason = lua_tolstring(l, 2, nullptr);
    if (!reason) {
        return luaL_error(l, "http_resp:set_reason() arg is NIL");
    }

    http_resp->reason = reason;
    BOOST_LOG_TRIVIAL(debug) << "lua: http_resp:set_reason(" << http_resp->reason << ")";
    return 0;
}

int set_body(lua_State* l){
    auto http_resp = my_check_type<http_resp_t>(l, 2, LUA_HTTP_RESP_META);
    if (!http_resp) return 1;

    size_t len = 0;
    const char* body = lua_tolstring(l, 2, &len);
    if (!body) {
        return luaL_error(l, "http_resp:set_body() arg is NIL");
    }

    http_resp->body.assign(body, len);
    BOOST_LOG_TRIVIAL(debug) << "lua: http_resp:set_body()";
    return 0;
}

int append_body(lua_State* l){
    auto http_resp = my_check_type<http_resp_t>(l, 2, LUA_HTTP_RESP_META);
    if (!http_resp) return 1;

    size_t len = 0;
    const char* body = lua_tolstring(l, 2, &len);
    if (!body) {
        return luaL_error(l, "http_resp:set_body() arg is NIL");
    }

    http_resp->body.append(body, len);
    BOOST_LOG_TRIVIAL(debug) << "lua: http_resp:append_body()";
    return 0;
}

int dump(lua_State* l) {
    auto http_resp = my_check_type<http_resp_t>(l, 1, LUA_HTTP_RESP_META);
    if (!http_resp) return 0;

    BOOST_LOG_TRIVIAL(debug) << "lua: http_resp:dump(): ptr=" << (void*)http_resp << "\n" << *http_resp;
    return 0;
}

void register_type(lua_State* l) {
    BOOST_LOG_TRIVIAL(debug) << "lua: register http_resp";
    static const luaL_Reg http_resp_meta[] = {
        {"new", new_msg},
        {"delete", del_msg},
        {"dump", dump},
        {"add_header", add_header},
        {"get_header", get_header},
        {"set_status_code", set_status_code},
        {"set_reason", set_reason},
        {"set_body", set_body},
        {"append_body", append_body},
        { nullptr, nullptr }
    };

    // luaL_newmetatable(l, LUA_HTTP_RESP_META);
    // lua_pushvalue(l, -1);
    // lua_setfield(l, -1, "__index");
    // luaL_newlib(l, http_resp_meta);
    // lua_setglobal(l, "proto_type");
    // luaL_getmetatable(l, LUA_HTTP_RESP_META);
    // luaL_setfuncs(l, http_resp_meta, 0);
    // luaL_newlib(l, meta_static);
    // luaL_newmetatable(l, LUA_HTTP_RESP_META);
    // luaL_setfuncs(l, http_resp_meta, 0);
    // lua_pushvalue(l, -1);
    // lua_setfield(l, -1, "__index");
    // luaL_newlib(l, http_resp_meta);
    // lua_setglobal(l, "http_resp");

    // create http_resp
    lua_createtable(l, 0, 0);
    int lib_id = lua_gettop(l);
    luaL_newmetatable(l, LUA_HTTP_RESP_META);
    int meta_id = lua_gettop(l);
    luaL_setfuncs(l, http_resp_meta, 0);

    luaL_newlib(l, http_resp_meta);
    lua_setfield(l, meta_id, "__index");

    luaL_newlib(l,http_resp_meta);
    lua_setfield(l, meta_id, "__metatable");

    lua_setmetatable(l, lib_id);
    lua_setglobal(l, "http_resp");

}

} //namespace http_resp

void register_custom_functions(lua_State* l) {
    BOOST_LOG_TRIVIAL(debug) << "lua: registering custom types";

    http_resp::register_type(l);
    http_req::register_type(l);
    shared_storage_t::register_type(l);
}
