cmake_minimum_required(VERSION 3.15)
project(idcu-microkernel C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-microkernel STATIC
    src/idcu/microkernel/kernel.c
)

target_include_directories(idcu-microkernel PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-microkernel PUBLIC
    idcu::common
    idcu::module-system
    idcu::msgbus
    idcu::coroutine
)

if(WIN32)
    target_link_libraries(idcu-microkernel PRIVATE)
else()
    target_link_libraries(idcu-microkernel PRIVATE pthread)
endif()

add_library(idcu::microkernel ALIAS idcu-microkernel)

# if(BUILD_TESTING)
#     add_subdirectory(tests)
# endif()

# if(BUILD_EXAMPLES)
#     add_subdirectory(examples)
# endif()
