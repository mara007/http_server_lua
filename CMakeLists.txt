project("server")
cmake_minimum_required(VERSION 3.22)

set(TARGET http_server)
add_executable(${TARGET} main.cpp)

set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         OFF) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS log log_setup)
# find_package(Boost)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
    target_link_libraries(${TARGET} ${Boost_LIBRARIES})
endif()

