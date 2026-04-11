include(idcu_module)

idcu_add_library(sdk_complete
    VERSION 1.0.0
    SOURCES
        src/sdk_complete.c
    HEADERS
    DEPENDS
        idcu::sdk
        idcu::coroutine
        idcu::module_system
        idcu::microkernel
)
