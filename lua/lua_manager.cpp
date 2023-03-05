#include "lua_manager.h"
#include "lua_http.h"

#include <cassert>
#include <algorithm>
#include <boost/log/trivial.hpp>


static const int LUA_TYPE_FUNCTION = 6;

lua_State* lua_manager_t::new_lua_state() {
    lua_State* l = luaL_newstate();
    luaL_openlibs(l);
    register_custom_functions(l);

    return l;
}

lua_manager_t::~lua_manager_t() {
    for(auto l : m_lua_states)
        lua_close(l);

    m_lua_states.clear();
    m_available_lua_states.clear();
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
            return false;
        }

        if (int t = lua_getglobal(l, "handle_http_message"); t != LUA_TYPE_FUNCTION) {
            BOOST_LOG_TRIVIAL(error) << "ERROR loading LUA script " << script << ", entry 'function "
                                    << LUA_ENTRY_FUNCTION << " handle_http_message(request, response)' must be defined in a script!";
            return false;
        }
    }

    return true;
}


bool lua_manager_t::invoke_script(const std::string& function_name, http_req_t* req, http_resp_t* resp) {
    lua_State* l = get_available_lua_state(true);

    BOOST_LOG_TRIVIAL(debug) << "SCRIPT ENTER: req=" << (void*)req << ", resp=" << (void*)resp;

    lua_getglobal(l, function_name.c_str());

    auto **req_ptr = (http_req_t **)lua_newuserdata(l, sizeof(http_req_t *));
    *req_ptr = req;
    luaL_setmetatable(l, LUA_HTTP_REQ_META);

    auto **resp_ptr = (http_resp_t **)lua_newuserdata(l, sizeof(http_resp_t *));
    *resp_ptr = resp;
    luaL_setmetatable(l, LUA_HTTP_RESP_META);


    bool lua_result = true;
    if (auto ret = lua_pcall(l, 2, 0, 0); ret != LUA_OK) {
        const char* lua_err_msg = lua_tolstring(l, -1, nullptr);
        BOOST_LOG_TRIVIAL(error) << "ERROR in lua script:" << lua_err_msg;
        lua_result = false;
    }

    return_lua_state_to_available(l);
    BOOST_LOG_TRIVIAL(debug) << "SCRIPT LEAVE";
    return lua_result;
}
