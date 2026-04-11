include(idcu_module)

idcu_add_library(network_integration
    VERSION 1.0.0
    SOURCES
        src/network_integration.c
    HEADERS
    DEPENDS
        idcu::sdk
        idcu::network
        idcu::conn_pool
        idcu::http_client
        idcu::http_server
        idcu::msgbus
        idcu::discovery
)
