include(idcu_module)

idcu_add_library(rest_api
    VERSION 1.0.0
    SOURCES
        src/idcu/rest_api/rest_api.c
    HEADERS
        include/idcu/rest_api/rest_api.h
        include/idcu/rest_api/types.h
    DEPENDS
        idcu::common
        idcu::http_server
        idcu::json
)
