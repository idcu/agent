include(idcu_module)

idcu_add_library(http_management_module
    VERSION 1.0.0
    SOURCES
        src/idcu/http_management_module/http_management_module.c
    HEADERS
        include/idcu/http_management_module/http_management_module.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::http_server
        idcu::management
        idcu::core_module
)
