cmake_minimum_required(VERSION 3.15)
project(idcu-coroutine VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" OFF)

set(SOURCES
    src/idcu/coroutine/coroutine.c
)

set(HEADERS
    include/idcu/coroutine/types.h
    include/idcu/coroutine/coroutine.h
)

add_library(idcu-coroutine STATIC ${SOURCES} ${HEADERS})

target_include_directories(idcu-coroutine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-coroutine
    PUBLIC
        idcu::common
)

add_library(idcu::coroutine ALIAS idcu-coroutine)

set_target_properties(idcu-coroutine PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
