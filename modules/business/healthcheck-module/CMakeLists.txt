include(idcu_module)

idcu_add_library(healthcheck_module
    VERSION 1.0.0
    SOURCES
        src/idcu/healthcheck_module/healthcheck_module.c
    HEADERS
        include/idcu/healthcheck_module/healthcheck_module.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::healthcheck
        idcu::core_module
)
