include(idcu_module)

idcu_add_library(core_module
    VERSION 1.0.0
    SOURCES
        src/idcu/core_module/core_module.c
    HEADERS
        include/idcu/core_module/core_module.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::log
        idcu::config
        idcu::healthcheck
        idcu::metrics
)
