#include "lua/lua_manager.h"
#include "http/http_message.h"

#include <iostream>

#include "gtest/gtest.h"

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

#define TEST_DIR_NAME STR(TEST_SRC_DIR)


TEST(lua_manager_t, load_script) {
    auto lua_man = lua_manager_t();
    ASSERT_TRUE(lua_man.init(TEST_DIR_NAME"/script.lua", 1));
}

TEST(lua_manager_t, invoke_script) {
    auto lua_man = lua_manager_t();
    ASSERT_TRUE(lua_man.init(TEST_DIR_NAME"/script.lua", 1));

    http_req_t http_req;
    http_req.method = "GET_TEST";
    http_req.path = "/PATH";

    http_resp_t http_resp(222, "OK_TEST");
    http_resp.add_header("TEST_HEADER", "TEST_VAL");

    ASSERT_TRUE(lua_man.invoke_script("handle_http_message", &http_req, &http_resp));

    ASSERT_EQ(http_req.get_header("PATH"), "/PATH");
    ASSERT_EQ(http_req.get_header("METHOD"), "GET_TEST");

    ASSERT_EQ(http_resp.get_header("TESTED_HEADER"), "TEST_VAL");
    ASSERT_EQ(http_resp.code, 200);
    ASSERT_EQ(http_resp.reason, "ITS OK");

}

TEST(shared_storage_t, store) {
    auto lua_man = lua_manager_t();
    ASSERT_TRUE(lua_man.init(TEST_DIR_NAME"/script.lua", 1));

    http_req_t http_req;
    http_resp_t http_resp(222, "OK_TEST");

    ASSERT_TRUE(lua_man.invoke_script("test_shared_storage", &http_req, &http_resp));

}

// ************* TEST ******************

struct test_lua_class_t {
    std::string dato_;
    const std::string& get_data() { return dato_; }
    void set_data(std::string data) { dato_ = data; }
};

int my_set(lua_State*l ) {
    test_lua_class_t *t = (test_lua_class_t*)lua_topointer(l, 1);
    auto s = luaL_checkstring(l, 2);
    t->set_data(s);
    return 0;
}

int my_get(lua_State*l ) {
    test_lua_class_t *t = (test_lua_class_t*)lua_topointer(l, 1);
    lua_pushstring(l, t->get_data().c_str());
    return 1;
}

int my_new(lua_State*l) {
    auto t = new test_lua_class_t();
    lua_pushlightuserdata(l, (void*)t);
    luaL_setmetatable(l, "my_meta");
    return 1;
}

int my_new2(lua_State*l) {
    auto **ptr = (test_lua_class_t **)lua_newuserdata(l, sizeof(test_lua_class_t *));
    *ptr = new test_lua_class_t();
    luaL_setmetatable(l, "my_meta2");
    return 1;
}

int my_set2(lua_State*l ) {
    auto **resp_ctx  = (test_lua_class_t **)luaL_checkudata(l, 1, "my_meta2");
    luaL_argcheck(l, resp_ctx != NULL, 1, "obj expected");

    auto s = luaL_checkstring(l, 2);
    (*resp_ctx)->set_data(s);
    return 0;
}

int my_get2(lua_State*l ) {
    auto **resp_ctx  = (test_lua_class_t **)luaL_checkudata(l, 1, "my_meta2");
    luaL_argcheck(l, resp_ctx != NULL, 1, "obj expected");

    lua_pushstring(l, (*resp_ctx)->get_data().c_str());
    return 1;
}

int foo_empty(lua_State*) { return 0;}

TEST(lua, prototype) {
    static const luaL_Reg meta_static[] = {
        {"new", my_new},
        {nullptr, nullptr}
    };
    static const luaL_Reg meta_inst[] = {
        {"new", my_new},
        {"get_data", my_get},
        {"set_data", my_set},
        {nullptr, nullptr}
    };
    static const luaL_Reg meta_inst2[] = {
        {"new2", my_new2},
        {"get_data2", my_get2},
        {"set_data2", my_set2},
        {nullptr, nullptr}
    };
    static const luaL_Reg _meta[] = {
    {"__gc", foo_empty},
    {"__index", foo_empty},
    {"__newindex", foo_empty},
    { NULL, NULL }
    };

    static const luaL_Reg _metas[] = {
    {"__gc", foo_empty},
    {"__index", foo_empty},
    {"__newindex", foo_empty},
    { NULL, NULL }
    };


    lua_State* l = luaL_newstate();
    auto ls = l;
    luaL_openlibs(l);


    // luaL_newmetatable(l, "my_meta2");
    // // lua_pushvalue(l, -1);
    // lua_setfield(l, -1, "__index");
    // luaL_newlib(l, meta_inst2);
    // lua_setglobal(l, "proto_type");
    // luaL_getmetatable(l, "my_meta2");
    // luaL_setfuncs(l, meta_inst2, 0);



    // luaL_newmetatable(l, "my_meta");
    // // lua_pushvalue(l, -1);
    // lua_setfield(l, -1, "__index");
    // luaL_newlib(l, meta_inst);
    // lua_setglobal(l, "proto_type");
    // luaL_getmetatable(l, "my_meta");
    // luaL_setfuncs(l, meta_inst, 0);

    int lib_id, meta_id;
    lua_createtable(ls, 0, 0);
    lib_id = lua_gettop(ls);
    luaL_newmetatable(ls, "my_meta");
    meta_id = lua_gettop(ls);
    luaL_setfuncs(ls, _meta, 0);

    luaL_newlib(ls, meta_inst);
    lua_setfield(ls, meta_id, "__index");

    luaL_newlib(ls, _metas);
    lua_setfield(ls, meta_id, "__metatable");

    lua_setmetatable(ls, lib_id);
    lua_setglobal(ls, "proto_type");


    lua_createtable(ls, 0, 0);
    lib_id = lua_gettop(ls);
    luaL_newmetatable(ls, "my_meta2");
    meta_id = lua_gettop(ls);
    luaL_setfuncs(ls, _meta, 0);

    luaL_newlib(ls, meta_inst2);
    lua_setfield(ls, meta_id, "__index");

    luaL_newlib(ls, _metas);
    lua_setfield(ls, meta_id, "__metatable");

    lua_setmetatable(ls, lib_id);
    lua_setglobal(ls, "proto_type2");




    if (int res = luaL_dofile(l, TEST_DIR_NAME"/script.lua"); res != LUA_OK) {
        BOOST_LOG_TRIVIAL(error) << "ERROR loading LUA script  error: " << lua_tostring(l, -1);
        lua_close(l);
        ASSERT_TRUE(res == LUA_OK);
    }

    lua_getglobal(l, "prototype_function");

    test_lua_class_t t, d;
    lua_pushlightuserdata(l, (void*)&t);
    luaL_setmetatable(l, "my_meta");

    test_lua_class_t **ptr = (test_lua_class_t **)lua_newuserdata(ls, sizeof(test_lua_class_t *));
    *ptr = &d;
    luaL_setmetatable(l, "my_meta2");



    if (auto ret = lua_pcall(l, 2, 0, 0); ret != LUA_OK) {
        const char* lua_err_msg = lua_tolstring(l, -1, nullptr);
        BOOST_LOG_TRIVIAL(error) << "ERROR in lua script:" << lua_err_msg;
        ASSERT_TRUE(ret == LUA_OK);
    }
    std::cout << "after script: " << t.get_data() << std::endl;
    std::cout << "after script: " << d.get_data() << std::endl;

}