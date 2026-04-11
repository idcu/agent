include(idcu_module)

idcu_add_library(basic_libs
    VERSION 1.0.0
    SOURCES
        src/basic_libs.c
    HEADERS
    DEPENDS
        idcu::sdk
        idcu::log
        idcu::config
        idcu::json
        idcu::yaml
        idcu::network
        idcu::metrics
)
