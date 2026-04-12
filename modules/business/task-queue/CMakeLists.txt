include(idcu_module)

idcu_add_library(task_queue
    VERSION 1.0.0
    SOURCES
        src/idcu/task_queue/task_queue.c
    HEADERS
        include/idcu/task_queue/task_queue.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::scheduler
        idcu::core_module
)
