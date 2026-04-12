include(idcu_module)

idcu_add_library(config_module
    VERSION 1.0.0
    SOURCES
        src/idcu/config_module/config_module.c
    HEADERS
        include/idcu/config_module/config_module.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::config
        idcu::yaml
        idcu::json
        idcu::msgbus
        idcu::core_module
)
