include(idcu_module)

idcu_add_library(http_client_module
    VERSION 1.0.0
    SOURCES
        src/idcu/http_client_module/http_client_module.c
    HEADERS
        include/idcu/http_client_module/http_client_module.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::http_client
        idcu::core_module
)
