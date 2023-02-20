#include "lua_manager.h"
#include "lua_functions.h"

#include <cassert>
#include <algorithm>
#include <boost/log/trivial.hpp>

static const char LUA_ENTRY_FUNCTION[] = "handle_http_message";

static const int LUA_TYPE_FUNCTION = 6;

lua_State* lua_manager_t::new_lua_state() {
    lua_State* l = luaL_newstate();
    luaL_openlibs(l);
    register_custom_functions(l);

    return l;
}


lua_State* lua_manager_t::get_available_lua_state(bool blocking) {
    if (std::unique_lock lock(m_available_lua_states_mutex); !m_available_lua_states.empty()) {
        auto result = m_available_lua_states.front();
        m_available_lua_states.pop_front();

        m_no_lua_state_available = m_available_lua_states.empty();

        return result;
    }

    if (!blocking)
        return nullptr;

    std::unique_lock lock(m_available_lua_states_mutex);
    m_available_lua_state_cv.wait(lock, [this]{ return !m_no_lua_state_available;});

    assert(!m_available_lua_states.empty());
    auto result = m_available_lua_states.front();
    m_available_lua_states.pop_front();
    m_no_lua_state_available = m_available_lua_states.empty();

    return result;
}

void lua_manager_t::return_lua_state_to_available(lua_State *l) {
    std::unique_lock lock(m_available_lua_states_mutex);
    m_available_lua_states.push_back(l);

    if (m_no_lua_state_available) {
        m_no_lua_state_available = false;
        m_available_lua_state_cv.notify_all();
    }
}

bool lua_manager_t::init(const std::string& script, size_t lua_instances) {
    BOOST_LOG_TRIVIAL(info) << "lua_manager_t::init script=" << script << ", lua_instances=" << lua_instances;

    for (size_t i = 0; i < lua_instances; ++i) {
        auto *l = new_lua_state();
        m_lua_states.push_back(l);
        m_available_lua_states.push_back(l);

        if (int res = luaL_dofile(l, script.c_str()); res != LUA_OK) {
            BOOST_LOG_TRIVIAL(error) << "ERROR loading LUA script " << script << ", error: " << lua_tostring(l, -1);
            lua_close(l);
            return false;
        }

        if (int t = lua_getglobal(l, "handle_http_message"); t != LUA_TYPE_FUNCTION) {
            BOOST_LOG_TRIVIAL(error) << "ERROR loading LUA script " << script << ", entry 'function "
                                    << LUA_ENTRY_FUNCTION << " handle_http_message(request, response)' must be defined in a script!";
            lua_close(l);
            return false;
        }
    }

    return true;
}


void lua_manager_t::invoke_script(http_req_t& req, http_resp_t& resp) {
    lua_State* l = get_available_lua_state(true);

    lua_getglobal(l, LUA_ENTRY_FUNCTION);
    lua_pushnumber(l, 1);
    lua_pushnumber(l, 2);

    lua_call(l, 2, 1);

    if (const char* res = luaL_checkstring(l, -1); res) {
        resp.code = 200;
        resp.body = res;
    }

    return_lua_state_to_available(l);
    BOOST_LOG_TRIVIAL(error) << "SCRIPT LEAVE";
}
