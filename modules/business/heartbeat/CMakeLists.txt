include(idcu_module)

idcu_add_library(heartbeat
    VERSION 1.0.0
    SOURCES
        src/idcu/heartbeat/heartbeat.c
    HEADERS
        include/idcu/heartbeat/heartbeat.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::network
        idcu::metrics
        idcu::core_module
)
