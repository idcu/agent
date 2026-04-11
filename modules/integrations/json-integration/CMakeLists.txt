include(idcu_module)

idcu_add_library(json_integration
    VERSION 1.0.0
    SOURCES
        src/json_integration.c
    HEADERS
    DEPENDS
        idcu::sdk
        idcu::json
)
