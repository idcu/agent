include(idcu_module)

idcu_add_library(collect_module
    VERSION 1.0.0
    SOURCES
        src/idcu/collect_module/collect_module.c
    HEADERS
        include/idcu/collect_module/collect_module.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::device_collector
        idcu::network
        idcu::core_module
)
