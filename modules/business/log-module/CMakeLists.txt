include(idcu_module)

idcu_add_library(log_module
    VERSION 1.0.0
    SOURCES
        src/idcu/log_module/log_module.c
    HEADERS
        include/idcu/log_module/log_module.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::log
        idcu::msgbus
        idcu::core_module
)
