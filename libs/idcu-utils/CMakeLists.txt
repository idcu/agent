cmake_minimum_required(VERSION 3.15)
project(idcu-utils C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-utils STATIC
    src/idcu/utils/utils.c
)

target_include_directories(idcu-utils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-utils PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-utils PRIVATE advapi32 shell32 shlwapi)
else()
    target_link_libraries(idcu-utils PRIVATE pthread)
endif()

add_library(idcu::utils ALIAS idcu-utils)
