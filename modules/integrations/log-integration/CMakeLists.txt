include(idcu_module)

idcu_add_library(log_integration
    VERSION 1.0.0
    SOURCES
        src/log_integration.c
    HEADERS
    DEPENDS
        idcu::sdk
        idcu::log
        idcu::config
)
