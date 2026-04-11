include(idcu_module)

idcu_add_library(yaml_integration
    VERSION 1.0.0
    SOURCES
        src/yaml_integration.c
    HEADERS
    DEPENDS
        idcu::sdk
        idcu::yaml
        idcu::json
)
