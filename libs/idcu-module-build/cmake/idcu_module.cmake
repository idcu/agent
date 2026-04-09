include_guard()

function(idcu_add_library name)
    cmake_parse_arguments(IDCU_LIB
        ""
        "VERSION"
        "SOURCES;HEADERS;DEPENDS"
        ${ARGN}
    )
    
    add_library(${name} STATIC ${IDCU_LIB_SOURCES})
    
    target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    
    if(IDCU_LIB_DEPENDS)
        target_link_libraries(${name} PUBLIC ${IDCU_LIB_DEPENDS})
    endif()
    
    add_library(idcu::${name} ALIAS ${name})
    
    message(STATUS "Added library: idcu::${name}")
endfunction()

function(idcu_add_module name)
    cmake_parse_arguments(IDCU_MODULE
        ""
        "VERSION"
        "SOURCES;DEPENDS"
        ${ARGN}
    )
    
    add_library(${name} STATIC ${IDCU_MODULE_SOURCES})
    
    target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
    
    if(IDCU_MODULE_DEPENDS)
        target_link_libraries(${name} PUBLIC ${IDCU_MODULE_DEPENDS})
    endif()
    
    message(STATUS "Added module: ${name}")
endfunction()
