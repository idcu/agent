include(idcu_module)

idcu_add_library(alert_module
    VERSION 1.0.0
    SOURCES
        src/idcu/alert_module/alert_module.c
    HEADERS
        include/idcu/alert_module/alert_module.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::alert
        idcu::core_module
)
