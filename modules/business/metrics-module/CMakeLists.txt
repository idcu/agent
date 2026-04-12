include(idcu_module)

idcu_add_library(metrics_module
    VERSION 1.0.0
    SOURCES
        src/idcu/metrics_module/metrics_module.c
    HEADERS
        include/idcu/metrics_module/metrics_module.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::metrics
        idcu::alert
        idcu::core_module
)
