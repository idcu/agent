include(idcu_module)

idcu_add_library(security_module
    VERSION 1.0.0
    SOURCES
        src/idcu/security_module/security_module.c
    HEADERS
        include/idcu/security_module/security_module.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::sandbox
        idcu::permission
        idcu::core_module
)
