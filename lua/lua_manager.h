#include "lua.hpp"
#include "http/http_message.h"

#include <mutex>
#include <deque>
#include <vector>
#include <condition_variable>


class lua_manager_t {

    static lua_State* new_lua_state();

    lua_State* get_available_lua_state(bool blocking);
    void return_lua_state_to_available(lua_State *l);

    std::vector<lua_State*> m_lua_states;
    std::deque<lua_State*> m_available_lua_states;
    std::mutex m_available_lua_states_mutex;
    std::condition_variable m_available_lua_state_cv;
    bool m_no_lua_state_available = false;

    public:

    bool init(const std::string& script, size_t lua_instances);

    void invoke_script(http_req_t& req, http_resp_t& resp);
};