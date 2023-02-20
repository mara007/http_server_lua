project("http server")
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_FLAGS "-std=c++17 -ggdb3" )


set(TARGET http_server)
add_executable(${TARGET} main.cpp)

set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         ON) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS log log_setup REQUIRED)
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

find_package(Lua 5.3 REQUIRED)
message(STATUS "LUA libraries: ${LUA_LIBRARIES}")
message(STATUS "LUA includes : ${LUA_INCLUDE_DIR}")

include_directories(${LUA_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(.)

target_link_libraries(
    ${TARGET}
    http
    lua_man
    ${Boost_LIBRARIES}
)

# target_compile_definitions(${TARGET} PRIVATE BOOST_ASIO_ENABLE_HANDLER_TRACKING=1)

add_subdirectory(http)
add_subdirectory(lua)

add_subdirectory(test)

