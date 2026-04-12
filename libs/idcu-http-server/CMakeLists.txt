cmake_minimum_required(VERSION 3.15)
project(idcu-http-server C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-http-server STATIC
    src/idcu/http_server/http_server.c
)

target_include_directories(idcu-http-server PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-http-server PUBLIC idcu::common idcu::network)

if(WIN32)
    target_link_libraries(idcu-http-server PRIVATE)
else()
    target_link_libraries(idcu-http-server PRIVATE pthread)
endif()

add_library(idcu::http-server ALIAS idcu-http-server)
add_library(idcu::http_server ALIAS idcu-http-server)

# if(BUILD_TESTS)
#     add_subdirectory(tests)
# endif()
