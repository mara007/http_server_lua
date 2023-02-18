project("http server")
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_FLAGS "-std=c++17 -ggdb3" )


set(TARGET http_server)
add_executable(${TARGET} main.cpp)

add_library(server
    server.cpp
    http.cpp
)
target_link_libraries(server ${Boost_LIBRARIES})

set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         ON) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS log log_setup REQUIRED)
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
set(BOOST_LIB ${Boost_LIBRARIES})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(.)

target_link_libraries(
    ${TARGET}
    server
    ${Boost_LIBRARIES}
)

add_subdirectory(test)

