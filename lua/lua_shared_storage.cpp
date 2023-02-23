#include "lua_shared_storage.h"

#include <boost/log/trivial.hpp>

shared_storage_t::shared_storage_t()
: m_mutex()
, m_map()
{}

shared_storage_t* shared_storage_t::instance() {
    static shared_storage_t m_instance;
    return &m_instance;
}

void shared_storage_t::put(std::string key, std::string val) {
    auto guard = std::lock_guard(m_mutex);
    m_map.emplace(std::pair{std::move(key), std::move(val)});
}

std::optional<std::string> shared_storage_t::get(const std::string& key) {
    auto guard = std::lock_guard(m_mutex);
    if (auto it = m_map.find(key); it != std::end(m_map)) {
        return it->second;
    }
    return std::nullopt;
}

size_t shared_storage_t::size() {
    auto guard = std::lock_guard(m_mutex);
    return m_map.size();
}

// *************** LUA ********************
int storage_put(lua_State* l) {
    if (lua_gettop(l) != 2) {
        luaL_error(l, "invalid number of arguments");
        return 1;
    }

    const char* key = luaL_checkstring(l, 1);
    const char* val = luaL_checkstring(l, 2);
    shared_storage_t::instance()->put(key, val);
    return 0;
}

int storage_get(lua_State* l) {
    if (lua_gettop(l) != 1) {
        luaL_error(l, "invalid number of arguments");
        return 1;
    }

    const char* key = luaL_checkstring(l, 1);
    if (auto val = shared_storage_t::instance()->get(key); val) {
        lua_pushlstring(l, val.value().data(), val.value().size());
    } else {
        lua_pushnil(l);
    }

    return 1;

}

int storage_size(lua_State* l) {
    if (lua_gettop(l) != 0) {
        luaL_error(l, "invalid number of arguments");
        return 1;
    }

    lua_pushinteger(l, shared_storage_t::instance()->size());
    return 1;
}


void shared_storage_t::register_type(lua_State* l) {
    BOOST_LOG_TRIVIAL(debug) << "lua: register shared_storage";
    static const luaL_Reg shared_storage_meta[] = {
        {"put", storage_put},
        {"get", storage_get},
        {"size", storage_size},
        {"__size", storage_size},
        { nullptr, nullptr }
    };

    // create http_req
    lua_createtable(l, 0, 0);
    int lib_id = lua_gettop(l);
    luaL_newmetatable(l, LUA_STORAGE_META);
    int meta_id = lua_gettop(l);
    luaL_setfuncs(l, shared_storage_meta, 0);

    luaL_newlib(l, shared_storage_meta);
    lua_setfield(l, meta_id, "__index");

    luaL_newlib(l,shared_storage_meta);
    lua_setfield(l, meta_id, "__metatable");

    lua_setmetatable(l, lib_id);
    lua_setglobal(l, "shared_storage");

    auto **storage = (shared_storage_t **)lua_newuserdata(l, sizeof(shared_storage_t *));
    *storage = instance();
    luaL_setmetatable(l, LUA_STORAGE_META);
    lua_setglobal(l, "SHARED_STORAGE");




}