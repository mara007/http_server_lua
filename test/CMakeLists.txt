find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})
include_directories(..)

file(GLOB TEST_FILES *.cpp)
enable_testing()
add_executable(test_http_server ${TEST_FILES})
target_compile_definitions(test_http_server PRIVATE TEST_SRC_DIR=${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(
    test_http_server
    http
    lua_man

    ${Boost_LIBRARIES}
    GTest::gtest GTest::gtest_main
)