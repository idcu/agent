cmake_minimum_required(VERSION 3.15)
project(idcu-common C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-common STATIC
    src/idcu/common/error_code.c
    src/idcu/common/vector.c
    src/idcu/common/hash_map.c
    src/idcu/common/string_buf.c
    src/idcu/common/lock.c
    src/idcu/common/atomic.c
    src/idcu/common/linked_list.c
    src/idcu/common/security.c
    src/idcu/common/option.c
    src/idcu/common/deadlock_detector.c
)

target_include_directories(idcu-common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(WIN32)
    target_link_libraries(idcu-common PRIVATE)
else()
    target_link_libraries(idcu-common PRIVATE pthread)
endif()

add_library(idcu::common ALIAS idcu-common)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
