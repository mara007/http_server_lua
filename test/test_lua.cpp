#include "lua/lua_manager.h"

#include <iostream>

#include "gtest/gtest.h"

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

#define TEST_DIR_NAME STR(TEST_SRC_DIR)


TEST(lua_manager_t, load_script) {
    auto lua_man = lua_manager_t();
    ASSERT_TRUE(lua_man.init(TEST_DIR_NAME"/script.lua", 1));
}