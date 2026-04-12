cmake_minimum_required(VERSION 3.15)
project(idcu-rest-api C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-rest-api STATIC
    src/idcu/rest_api/rest_api.c
)

target_include_directories(idcu-rest-api PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-rest-api PUBLIC
    idcu::common
    idcu::http-server
    idcu::json
)

if(WIN32)
    target_link_libraries(idcu-rest-api PRIVATE ws2_32)
else()
    target_link_libraries(idcu-rest-api PRIVATE pthread)
endif()

add_library(idcu::rest-api ALIAS idcu-rest-api)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
