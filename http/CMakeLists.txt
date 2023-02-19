include_directories(.)

add_library(http
    http_connection.cpp
    http_message.cpp
)

target_link_libraries(http ${Boost_LIBRARIES})
