cmake_minimum_required(VERSION 3.15)
project(idcu-task-queue C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-task-queue STATIC
    src/idcu/task_queue/task_queue.c
)

target_include_directories(idcu-task-queue PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-task-queue PUBLIC
    idcu::common
    idcu::scheduler
    idcu::core_module
)

if(WIN32)
    target_link_libraries(idcu-task-queue PRIVATE)
else()
    target_link_libraries(idcu-task-queue PRIVATE pthread)
endif()

add_library(idcu::task_queue ALIAS idcu-task-queue)
