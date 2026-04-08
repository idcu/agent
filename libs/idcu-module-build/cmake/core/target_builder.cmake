# 目标构建器模块
# 负责创建库/可执行文件目标并配置

include("${CMAKE_CURRENT_LIST_DIR}/platform_detection.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/json_utils.cmake")

# 创建并配置构建目标
function(create_build_target MODULE_NAME MODULE_DIR)
    cmake_parse_arguments(
        CREATE_TARGET
        ""
        "LIB_TYPE;LANG_STANDARD;ALIAS_NAMESPACE;ALIAS_PREFIXES"
        "SOURCES;LINK_LIBS;INCLUDES;PLATFORM_DEPS"
        ${ARGN}
    )

    # 设置 C 标准
    set(CMAKE_C_STANDARD ${CREATE_TARGET_LANG_STANDARD})
    set(CMAKE_C_STANDARD_REQUIRED ON)

    # 创建目标
    set(IS_EXECUTABLE OFF)
    set(IS_SHARED_LIBRARY OFF)
    
    if(CREATE_TARGET_LIB_TYPE STREQUAL "executable")
        add_executable(${MODULE_NAME} ${CREATE_TARGET_SOURCES})
        set(IS_EXECUTABLE ON)
    elseif(CREATE_TARGET_LIB_TYPE STREQUAL "shared")
        add_library(${MODULE_NAME} SHARED ${CREATE_TARGET_SOURCES})
        set(IS_SHARED_LIBRARY ON)
    else()
        add_library(${MODULE_NAME} STATIC ${CREATE_TARGET_SOURCES})
    endif()

    # 跨平台编译选项
    if(WIN32)
        # Windows 特定选项
        target_compile_definitions(${MODULE_NAME} PRIVATE
            _CRT_SECURE_NO_WARNINGS
            _CRT_NONSTDC_NO_DEPRECATE
            WIN32_LEAN_AND_MEAN
        )
        
        if(MSVC)
            # MSVC 特定编译选项
            target_compile_options(${MODULE_NAME} PRIVATE
                /W3
                /utf-8
            )
            if(NOT IS_EXECUTABLE)
                target_compile_options(${MODULE_NAME} PRIVATE
                    /EHsc
                )
            endif()
        elseif(MINGW)
            # MinGW 特定编译选项
            target_compile_options(${MODULE_NAME} PRIVATE
                -Wall
                -Wextra
                -Wno-unused-parameter
            )
        endif()
        
        # Windows 动态库导出定义
        if(IS_SHARED_LIBRARY)
            string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)
            target_compile_definitions(${MODULE_NAME} PRIVATE
                ${MODULE_NAME_UPPER}_EXPORTS
            )
        endif()
        
    elseif(APPLE)
        # macOS 特定选项
        set_target_properties(${MODULE_NAME} PROPERTIES
            MACOSX_RPATH ON
            INSTALL_NAME_DIR "@rpath"
        )
        
        target_compile_options(${MODULE_NAME} PRIVATE
            -Wall
            -Wextra
            -Wno-unused-parameter
        )
        
        if(NOT IS_EXECUTABLE)
            set_target_properties(${MODULE_NAME} PROPERTIES
                BUILD_WITH_INSTALL_RPATH ON
            )
        endif()
        
    elseif(UNIX)
        # Linux/Unix 特定选项
        target_compile_options(${MODULE_NAME} PRIVATE
            -Wall
            -Wextra
            -Wno-unused-parameter
            -fPIC
        )
        
        if(NOT IS_EXECUTABLE)
            set_target_properties(${MODULE_NAME} PROPERTIES
                POSITION_INDEPENDENT_CODE ON
                INSTALL_RPATH "$ORIGIN"
                BUILD_WITH_INSTALL_RPATH ON
            )
        endif()
    endif()

    # 配置头文件目录
    if(CREATE_TARGET_INCLUDES)
        foreach(INC_PATH ${CREATE_TARGET_INCLUDES})
            target_include_directories(${MODULE_NAME} PUBLIC
                $<BUILD_INTERFACE:${MODULE_DIR}/${INC_PATH}>
            )
        endforeach()
    else()
        if(EXISTS "${MODULE_DIR}/include")
            target_include_directories(${MODULE_NAME} PUBLIC
                $<BUILD_INTERFACE:${MODULE_DIR}/include>
                $<INSTALL_INTERFACE:include>
            )
        endif()
    endif()

    # 链接依赖
    foreach(DEP_NAME ${CREATE_TARGET_LINK_LIBS})
        if(CREATE_TARGET_ALIAS_NAMESPACE)
            if(TARGET ${CREATE_TARGET_ALIAS_NAMESPACE}::${DEP_NAME})
                target_link_libraries(${MODULE_NAME} PUBLIC ${CREATE_TARGET_ALIAS_NAMESPACE}::${DEP_NAME})
            elseif(TARGET ${DEP_NAME})
                target_link_libraries(${MODULE_NAME} PUBLIC ${DEP_NAME})
            endif()
        elseif(TARGET ${DEP_NAME})
            target_link_libraries(${MODULE_NAME} PUBLIC ${DEP_NAME})
        endif()
    endforeach()

    # 处理平台依赖
    if(CREATE_TARGET_PLATFORM_DEPS)
        if(WIN32)
            safe_json_get(WIN32_LIBS "${CREATE_TARGET_PLATFORM_DEPS}" KEYS "win32")
            if(WIN32_LIBS)
                string(JSON WIN32_LIB_COUNT LENGTH "${WIN32_LIBS}")
                if(WIN32_LIB_COUNT GREATER 0)
                    math(EXPR WIN32_LIB_COUNT "${WIN32_LIB_COUNT} - 1")
                    foreach(LIB_INDEX RANGE ${WIN32_LIB_COUNT})
                        string(JSON LIB_NAME GET "${WIN32_LIBS}" ${LIB_INDEX})
                        target_link_libraries(${MODULE_NAME} PUBLIC ${LIB_NAME})
                    endforeach()
                endif()
            endif()
        else()
            safe_json_get(UNIX_LIBS "${CREATE_TARGET_PLATFORM_DEPS}" KEYS "unix")
            if(UNIX_LIBS)
                string(JSON UNIX_LIB_COUNT LENGTH "${UNIX_LIBS}")
                if(UNIX_LIB_COUNT GREATER 0)
                    math(EXPR UNIX_LIB_COUNT "${UNIX_LIB_COUNT} - 1")
                    foreach(LIB_INDEX RANGE ${UNIX_LIB_COUNT})
                        string(JSON LIB_NAME GET "${UNIX_LIBS}" ${LIB_INDEX})
                        target_link_libraries(${MODULE_NAME} PUBLIC ${LIB_NAME})
                    endforeach()
                endif()
            endif()
        endif()
    endif()

    # 创建别名
    if(CREATE_TARGET_ALIAS_NAMESPACE AND NOT IS_EXECUTABLE)
        set(MODULE_SHORT_NAMES "")
        
        if(CREATE_TARGET_ALIAS_PREFIXES)
            foreach(PREFIX ${CREATE_TARGET_ALIAS_PREFIXES})
                string(REGEX REPLACE "^${PREFIX}[-_]" "" SHORT_NAME "${MODULE_NAME}")
                if(NOT SHORT_NAME STREQUAL MODULE_NAME)
                    list(APPEND MODULE_SHORT_NAMES "${SHORT_NAME}")
                endif()
            endforeach()
        else()
            list(APPEND MODULE_SHORT_NAMES "${MODULE_NAME}")
        endif()
        
        foreach(SHORT_NAME ${MODULE_SHORT_NAMES})
            if(NOT TARGET ${CREATE_TARGET_ALIAS_NAMESPACE}::${SHORT_NAME})
                add_library(${CREATE_TARGET_ALIAS_NAMESPACE}::${SHORT_NAME} ALIAS ${MODULE_NAME})
            endif()
        endforeach()
    endif()

endfunction()
