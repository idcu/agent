cmake_minimum_required(VERSION 3.15)
project(idcu-module-system VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" OFF)

set(SOURCES
    src/idcu/module/module.c
)

set(HEADERS
    include/idcu/module/types.h
    include/idcu/module/module.h
)

add_library(idcu-module-system STATIC ${SOURCES} ${HEADERS})

target_include_directories(idcu-module-system
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-module-system
    PUBLIC
        idcu::common
)

add_library(idcu::module-system ALIAS idcu-module-system)
add_library(idcu::module_system ALIAS idcu-module-system)

set_target_properties(idcu-module-system PROPERTIES
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
