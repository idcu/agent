cmake_minimum_required(VERSION 3.15)
project(idcu-module-verifier C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-module-verifier STATIC
    src/idcu/module_verifier/module_verifier.c
)

target_include_directories(idcu-module-verifier PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-module-verifier PUBLIC idcu::common)

if(WIN32)
    target_link_libraries(idcu-module-verifier PRIVATE)
else()
    target_link_libraries(idcu-module-verifier PRIVATE pthread)
endif()

add_library(idcu::module-verifier ALIAS idcu-module-verifier)
add_library(idcu::module_verifier ALIAS idcu-module-verifier)
