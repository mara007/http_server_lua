find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})
include_directories(..)

file(GLOB TEST_FILES *.cpp)
enable_testing()
add_executable(test_http_server ${TEST_FILES})

target_link_libraries(
    test_http_server
    http
    ${Boost_LIBRARIES}
    GTest::gtest GTest::gtest_main
)