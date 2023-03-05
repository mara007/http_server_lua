#pragma once

#include "lua.hpp"


static const char* LUA_HTTP_RESP_META = "http_resp.meta";
static const char* LUA_HTTP_REQ_META  = "http_req.meta";

void register_custom_functions(lua_State *l);