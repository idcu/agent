cmake_minimum_required(VERSION 3.15)
project(idcu-msgbus C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-msgbus STATIC
    src/idcu/msgbus/msgbus.c
)

target_include_directories(idcu-msgbus PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-msgbus PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-msgbus PRIVATE)
else()
    target_link_libraries(idcu-msgbus PRIVATE pthread)
endif()

add_library(idcu::msgbus ALIAS idcu-msgbus)

# if(BUILD_TESTING)
#     add_subdirectory(tests)
# endif()

# if(BUILD_EXAMPLES)
#     add_subdirectory(examples)
# endif()
