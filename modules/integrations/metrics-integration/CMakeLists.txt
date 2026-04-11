include(idcu_module)

idcu_add_library(metrics_integration
    VERSION 1.0.0
    SOURCES
        src/metrics_integration.c
    HEADERS
    DEPENDS
        idcu::sdk
        idcu::metrics
        idcu::alert
        idcu::storage
)
