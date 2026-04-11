include(idcu_module)

idcu_add_library(config_integration
    VERSION 1.0.0
    SOURCES
        src/config_integration.c
    HEADERS
    DEPENDS
        idcu::sdk
        idcu::config
        idcu::yaml
        idcu::json
)
