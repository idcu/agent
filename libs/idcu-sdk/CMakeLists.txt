cmake_minimum_required(VERSION 3.15)
project(idcu-sdk C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-sdk STATIC
    src/idcu/sdk/sdk.c
)

target_include_directories(idcu-sdk PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-sdk PUBLIC
    idcu::common
    idcu::microkernel
)

if(WIN32)
    target_link_libraries(idcu-sdk PRIVATE)
else()
    target_link_libraries(idcu-sdk PRIVATE pthread)
endif()

add_library(idcu::sdk ALIAS idcu-sdk)

# if(BUILD_TESTING)
#     add_subdirectory(tests)
# endif()

# if(BUILD_EXAMPLES)
#     add_subdirectory(examples)
# endif()
