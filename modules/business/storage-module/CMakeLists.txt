include(idcu_module)

idcu_add_library(storage_module
    VERSION 1.0.0
    SOURCES
        src/idcu/storage_module/storage_module.c
    HEADERS
        include/idcu/storage_module/storage_module.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::storage
        idcu::core_module
)
