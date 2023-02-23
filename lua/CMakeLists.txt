
include_directories(.)

add_library(lua_man
    lua_manager.cpp
    lua_functions.cpp
    lua_shared_storage.cpp
)

target_link_libraries(lua_man ${LUA_LIBRARIES})
