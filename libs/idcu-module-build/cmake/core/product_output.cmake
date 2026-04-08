# Product Output - 产物标准化输出模块
# 负责产物的标准化输出和打包

include("${CMAKE_CURRENT_LIST_DIR}/platform_detection.cmake")

# 生成 build-info.json 文件
function(_generate_build_info)
    cmake_parse_arguments(
        GEN_BUILD_INFO
        ""
        "OUTPUT_DIR;MODULE_NAME;MODULE_VERSION"
        ""
        ${ARGN}
    )

    set(BUILD_INFO_PATH "${GEN_BUILD_INFO_OUTPUT_DIR}/build-info.json")
    
    # 获取当前时间
    string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%dT%H:%M:%SZ" UTC)
    
    # 获取Git信息（如果可用）
    set(GIT_COMMIT "unknown")
    set(GIT_BRANCH "unknown")
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
        find_package(Git QUIET)
        if(GIT_FOUND)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                OUTPUT_VARIABLE GIT_COMMIT
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                OUTPUT_VARIABLE GIT_BRANCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        endif()
    endif()
    
    # 构建 build-info.json 内容
    set(BUILD_INFO_CONTENT "{\n")
    string(APPEND BUILD_INFO_CONTENT "  \"module\": {\n")
    string(APPEND BUILD_INFO_CONTENT "    \"name\": \"${GEN_BUILD_INFO_MODULE_NAME}\",\n")
    string(APPEND BUILD_INFO_CONTENT "    \"version\": \"${GEN_BUILD_INFO_MODULE_VERSION}\"\n")
    string(APPEND BUILD_INFO_CONTENT "  },\n")
    string(APPEND BUILD_INFO_CONTENT "  \"build\": {\n")
    string(APPEND BUILD_INFO_CONTENT "    \"timestamp\": \"${BUILD_TIMESTAMP}\",\n")
    string(APPEND BUILD_INFO_CONTENT "    \"platform\": \"${PLATFORM_NAME}\",\n")
    string(APPEND BUILD_INFO_CONTENT "    \"arch\": \"${ARCH_NAME}\",\n")
    string(APPEND BUILD_INFO_CONTENT "    \"compiler\": \"${COMPILER_NAME}\",\n")
    string(APPEND BUILD_INFO_CONTENT "    \"cmake_version\": \"${CMAKE_VERSION}\"\n")
    string(APPEND BUILD_INFO_CONTENT "  },\n")
    string(APPEND BUILD_INFO_CONTENT "  \"git\": {\n")
    string(APPEND BUILD_INFO_CONTENT "    \"commit\": \"${GIT_COMMIT}\",\n")
    string(APPEND BUILD_INFO_CONTENT "    \"branch\": \"${GIT_BRANCH}\"\n")
    string(APPEND BUILD_INFO_CONTENT "  }\n")
    string(APPEND BUILD_INFO_CONTENT "}\n")
    
    # 写入文件
    file(WRITE "${BUILD_INFO_PATH}" "${BUILD_INFO_CONTENT}")
    message(STATUS "Generated build-info.json: ${BUILD_INFO_PATH}")
endfunction()

# 标准化产物输出
function(package_module)
    cmake_parse_arguments(
        PACKAGE_MODULE
        ""
        "MODULE_DIR;MODULE_NAME;MODULE_VERSION;OUTPUT_ROOT"
        ""
        ${ARGN}
    )

    if(NOT PACKAGE_MODULE_MODULE_DIR)
        set(PACKAGE_MODULE_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    if(NOT PACKAGE_MODULE_MODULE_NAME)
        message(FATAL_ERROR "MODULE_NAME is required for package_module")
    endif()

    if(NOT PACKAGE_MODULE_MODULE_VERSION)
        set(PACKAGE_MODULE_MODULE_VERSION "1.0.0")
    endif()

    if(NOT PACKAGE_MODULE_OUTPUT_ROOT)
        set(PACKAGE_MODULE_OUTPUT_ROOT "${CMAKE_BINARY_DIR}/output")
    endif()

    # 执行 pre_package 钩子
    execute_hook(
        HOOK_NAME "pre_package"
        MODULE_DIR "${PACKAGE_MODULE_MODULE_DIR}"
        MODULE_NAME "${PACKAGE_MODULE_MODULE_NAME}"
        MODULE_VERSION "${PACKAGE_MODULE_MODULE_VERSION}"
    )

    # 创建输出目录结构：{module}-{version}-{platform}-{arch}/
    set(PACKAGE_NAME "${PACKAGE_MODULE_MODULE_NAME}-${PACKAGE_MODULE_MODULE_VERSION}-${PLATFORM_NAME}-${ARCH_NAME}")
    set(PACKAGE_OUTPUT_DIR "${PACKAGE_MODULE_OUTPUT_ROOT}/${PACKAGE_NAME}")
    set(PACKAGE_BIN_DIR "${PACKAGE_OUTPUT_DIR}/bin")
    set(PACKAGE_LIB_DIR "${PACKAGE_OUTPUT_DIR}/lib")
    set(PACKAGE_META_DIR "${PACKAGE_OUTPUT_DIR}/meta")

    # 创建目录
    file(MAKE_DIRECTORY "${PACKAGE_OUTPUT_DIR}")
    file(MAKE_DIRECTORY "${PACKAGE_BIN_DIR}")
    file(MAKE_DIRECTORY "${PACKAGE_LIB_DIR}")
    file(MAKE_DIRECTORY "${PACKAGE_META_DIR}")

    message(STATUS "Packaging module: ${PACKAGE_MODULE_MODULE_NAME} v${PACKAGE_MODULE_MODULE_VERSION}")
    message(STATUS "Output directory: ${PACKAGE_OUTPUT_DIR}")

    # 复制 module.json 到 meta/ 目录
    set(MODULE_JSON_SOURCE "${PACKAGE_MODULE_MODULE_DIR}/module.json")
    if(EXISTS "${MODULE_JSON_SOURCE}")
        configure_file("${MODULE_JSON_SOURCE}" "${PACKAGE_META_DIR}/module.json" COPYONLY)
        message(STATUS "Copied module.json to meta/ directory")
    endif()

    # 生成 build-info.json
    _generate_build_info(
        OUTPUT_DIR "${PACKAGE_OUTPUT_DIR}"
        MODULE_NAME "${PACKAGE_MODULE_MODULE_NAME}"
        MODULE_VERSION "${PACKAGE_MODULE_MODULE_VERSION}"
    )

    # 设置安装目标
    set_target_properties(${PACKAGE_MODULE_MODULE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PACKAGE_BIN_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${PACKAGE_LIB_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${PACKAGE_LIB_DIR}"
    )

    # 添加自定义目标用于后期打包（可扩展）
    add_custom_target(package_${PACKAGE_MODULE_MODULE_NAME}
        COMMAND ${CMAKE_COMMAND} -E echo "Module ${PACKAGE_MODULE_MODULE_NAME} packaged successfully at: ${PACKAGE_OUTPUT_DIR}"
        DEPENDS ${PACKAGE_MODULE_MODULE_NAME}
        COMMENT "Packaging module: ${PACKAGE_MODULE_MODULE_NAME}"
    )

    message(STATUS "Module packaging configured: ${PACKAGE_NAME}")

    # 执行 post_package 钩子
    execute_hook(
        HOOK_NAME "post_package"
        MODULE_DIR "${PACKAGE_MODULE_MODULE_DIR}"
        MODULE_NAME "${PACKAGE_MODULE_MODULE_NAME}"
        MODULE_VERSION "${PACKAGE_MODULE_MODULE_VERSION}"
    )
endfunction()

# 简化版：在 build_module 之后自动调用
function(auto_package_module)
    cmake_parse_arguments(
        AUTO_PACKAGE
        ""
        "MODULE_DIR;MODULE_NAME;MODULE_VERSION;OUTPUT_ROOT"
        ""
        ${ARGN}
    )

    if(NOT AUTO_PACKAGE_MODULE_NAME)
        message(FATAL_ERROR "MODULE_NAME is required for auto_package_module")
    endif()

    # 调用 package_module
    package_module(
        MODULE_DIR "${AUTO_PACKAGE_MODULE_DIR}"
        MODULE_NAME "${AUTO_PACKAGE_MODULE_NAME}"
        MODULE_VERSION "${AUTO_PACKAGE_MODULE_VERSION}"
        OUTPUT_ROOT "${AUTO_PACKAGE_OUTPUT_ROOT}"
    )
endfunction()
