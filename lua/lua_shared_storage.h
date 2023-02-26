#pragma once

#include <map>
#include <mutex>
#include <string>
#include <optional>
#include <vector>

#include "lua.hpp"

static const char LUA_STORAGE_META[] = "storage.meta";

class shared_storage_t {
    std::mutex m_mutex;
    std::map<std::string, std::string> m_map;

    shared_storage_t();
    public:

    static shared_storage_t* instance();

    void put(std::string key, std::string val);
    std::optional<std::string> get(const std::string& key);
    void del(const std::string& key);
    size_t size();
    std::vector<std::string> keys();

    static void register_type(lua_State* l);
};