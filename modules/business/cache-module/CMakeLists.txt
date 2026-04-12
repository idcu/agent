include(idcu_module)

idcu_add_library(cache_module
    VERSION 1.0.0
    SOURCES
        src/idcu/cache_module/cache_module.c
    HEADERS
        include/idcu/cache_module/cache_module.h
    DEPENDS
        idcu::sdk
        idcu::common
        idcu::cache
        idcu::core_module
)
