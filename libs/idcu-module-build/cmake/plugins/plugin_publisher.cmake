# Publisher Plugin - 发布插件
# 将构建产物发布到远程仓库

include(CMakeParseArguments)

# 注册插件
register_plugin(
    NAME "publisher"
    DESCRIPTION "Publish built modules to remote repositories"
    VERSION "1.0.0"
    AUTHOR "IDCU Team"
    HOOKS "post_package"
    DEPENDS "packager"
)

# 查找发布工具
function(_find_publish_tools RESULT_VAR)
    set(TOOLS "")

    # 查找 curl
    find_program(CURL_EXECUTABLE curl)
    if(CURL_EXECUTABLE)
        list(APPEND TOOLS "curl")
    endif()

    # 查找 scp
    find_program(SCP_EXECUTABLE scp)
    if(SCP_EXECUTABLE)
        list(APPEND TOOLS "scp")
    endif()

    # 查找 git
    find_package(Git QUIET)
    if(GIT_FOUND)
        list(APPEND TOOLS "git")
    endif()

    set(${RESULT_VAR} "${TOOLS}" PARENT_SCOPE)
endfunction()

# 使用 HTTP/HTTPS 发布
function(_publish_via_http)
    cmake_parse_arguments(
        PUBLISH_HTTP
        ""
        "FILE;URL;USERNAME;PASSWORD;TOKEN;METHOD"
        "HEADERS"
        ${ARGN}
    )

    if(NOT PUBLISH_HTTP_FILE)
        message(WARNING "[publisher] No file specified for HTTP publish")
        return()
    endif()

    if(NOT PUBLISH_HTTP_URL)
        message(WARNING "[publisher] No URL specified for HTTP publish")
        return()
    endif()

    if(NOT PUBLISH_HTTP_METHOD)
        set(PUBLISH_HTTP_METHOD "PUT")
    endif()

    find_program(CURL_EXECUTABLE curl)
    if(NOT CURL_EXECUTABLE)
        message(WARNING "[publisher] curl not found for HTTP publish")
        return()
    endif()

    message(STATUS "[publisher] Publishing via HTTP: ${PUBLISH_HTTP_FILE} -> ${PUBLISH_HTTP_URL}")

    set(CURL_ARGS "-X" "${PUBLISH_HTTP_METHOD}" "--upload-file" "${PUBLISH_HTTP_FILE}")

    # 添加认证
    if(PUBLISH_HTTP_TOKEN)
        list(APPEND CURL_ARGS "-H" "Authorization: Bearer ${PUBLISH_HTTP_TOKEN}")
    elseif(PUBLISH_HTTP_USERNAME AND PUBLISH_HTTP_PASSWORD)
        list(APPEND CURL_ARGS "-u" "${PUBLISH_HTTP_USERNAME}:${PUBLISH_HTTP_PASSWORD}")
    endif()

    # 添加自定义头
    foreach(HEADER ${PUBLISH_HTTP_HEADERS})
        list(APPEND CURL_ARGS "-H" "${HEADER}")
    endforeach()

    list(APPEND CURL_ARGS "${PUBLISH_HTTP_URL}")

    execute_process(
        COMMAND ${CURL_EXECUTABLE} ${CURL_ARGS}
        RESULT_VARIABLE CURL_RESULT
        OUTPUT_VARIABLE CURL_OUTPUT
        ERROR_VARIABLE CURL_ERROR
    )

    if(CURL_RESULT EQUAL 0)
        message(STATUS "[publisher] HTTP publish successful")
    else()
        message(WARNING "[publisher] HTTP publish failed: ${CURL_ERROR}")
    endif()
endfunction()

# 使用 SCP/SFTP 发布
function(_publish_via_scp)
    cmake_parse_arguments(
        PUBLISH_SCP
        ""
        "FILE;HOST;USER;PORT;PATH;KEY_FILE"
        ""
        ${ARGN}
    )

    if(NOT PUBLISH_SCP_FILE)
        message(WARNING "[publisher] No file specified for SCP publish")
        return()
    endif()

    if(NOT PUBLISH_SCP_HOST)
        message(WARNING "[publisher] No host specified for SCP publish")
        return()
    endif()

    find_program(SCP_EXECUTABLE scp)
    if(NOT SCP_EXECUTABLE)
        message(WARNING "[publisher] scp not found for SCP publish")
        return()
    endif()

    if(NOT PUBLISH_SCP_PORT)
        set(PUBLISH_SCP_PORT "22")
    endif()

    # 构建目标路径
    set(DESTINATION "")
    if(PUBLISH_SCP_USER)
        set(DESTINATION "${PUBLISH_SCP_USER}@")
    endif()
    string(APPEND DESTINATION "${PUBLISH_SCP_HOST}")
    if(PUBLISH_SCP_PATH)
        string(APPEND DESTINATION ":${PUBLISH_SCP_PATH}")
    endif()

    message(STATUS "[publisher] Publishing via SCP: ${PUBLISH_SCP_FILE} -> ${DESTINATION}")

    set(SCP_ARGS "-P" "${PUBLISH_SCP_PORT}")

    if(PUBLISH_SCP_KEY_FILE)
        list(APPEND SCP_ARGS "-i" "${PUBLISH_SCP_KEY_FILE}")
    endif()

    list(APPEND SCP_ARGS "${PUBLISH_SCP_FILE}" "${DESTINATION}")

    execute_process(
        COMMAND ${SCP_EXECUTABLE} ${SCP_ARGS}
        RESULT_VARIABLE SCP_RESULT
        OUTPUT_VARIABLE SCP_OUTPUT
        ERROR_VARIABLE SCP_ERROR
    )

    if(SCP_RESULT EQUAL 0)
        message(STATUS "[publisher] SCP publish successful")
    else()
        message(WARNING "[publisher] SCP publish failed: ${SCP_ERROR}")
    endif()
endfunction()

# 钩子分发函数
function(publisher_dispatch_hook)
    cmake_parse_arguments(
        DISPATCH
        ""
        "HOOK_NAME;MODULE_DIR;MODULE_NAME;MODULE_VERSION"
        ""
        ${ARGN}
    )

    if(DISPATCH_HOOK_NAME STREQUAL "post_package")
        publisher_post_package(
            MODULE_DIR "${DISPATCH_MODULE_DIR}"
            MODULE_NAME "${DISPATCH_MODULE_NAME}"
            MODULE_VERSION "${DISPATCH_MODULE_VERSION}"
        )
    endif()
endfunction()

# post_package 钩子实现
function(publisher_post_package)
    cmake_parse_arguments(
        PLUGIN_HOOK
        ""
        "MODULE_DIR;MODULE_NAME;MODULE_VERSION"
        ""
        ${ARGN}
    )

    message(STATUS "[publisher] Running post_package hook for ${PLUGIN_HOOK_MODULE_NAME}")

    # 从 module.json 读取发布配置
    set(MODULE_JSON_PATH "${PLUGIN_HOOK_MODULE_DIR}/module.json")

    if(NOT EXISTS "${MODULE_JSON_PATH}")
        message(STATUS "[publisher] No module.json found, skipping publish")
        return()
    endif()

    file(READ "${MODULE_JSON_PATH}" MODULE_JSON_CONTENT)

    # 检查是否有 publisher 配置
    string(JSON PUBLISHER_ERROR ERROR_VARIABLE GET "${MODULE_JSON_CONTENT}" "publisher")
    if(PUBLISHER_ERROR)
        message(STATUS "[publisher] No publisher configuration found")
        return()
    endif()

    string(JSON PUBLISHER_CONFIG GET "${MODULE_JSON_CONTENT}" "publisher")

    # 查找要发布的文件
    set(FILES_TO_PUBLISH "")
    set(PACKAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")

    # 查找归档文件
    file(GLOB ARCHIVE_FILES
        "${CMAKE_BINARY_DIR}/${PLUGIN_HOOK_MODULE_NAME}-*.zip"
        "${CMAKE_BINARY_DIR}/${PLUGIN_HOOK_MODULE_NAME}-*.tar.gz"
        "${CMAKE_BINARY_DIR}/${PLUGIN_HOOK_MODULE_NAME}-*.tar.bz2"
        "${CMAKE_BINARY_DIR}/${PLUGIN_HOOK_MODULE_NAME}-*.tar.xz"
    )
    list(APPEND FILES_TO_PUBLISH ${ARCHIVE_FILES})

    # 如果没有归档文件，查找输出目录
    if(NOT FILES_TO_PUBLISH)
        file(GLOB_RECURSE OUTPUT_FILES
            "${PACKAGE_OUTPUT_DIR}/*"
        )
        list(APPEND FILES_TO_PUBLISH ${OUTPUT_FILES})
    endif()

    if(NOT FILES_TO_PUBLISH)
        message(WARNING "[publisher] No files found to publish")
        return()
    endif()

    message(STATUS "[publisher] Found files to publish")

    # 查找可用工具
    _find_publish_tools(PUBLISH_TOOLS)
    message(STATUS "[publisher] Available tools: ${PUBLISH_TOOLS}")

    # 解析发布目标配置
    string(JSON TARGETS GET "${PUBLISHER_CONFIG}" "targets")
    if(NOT TARGETS)
        message(STATUS "[publisher] No publish targets configured")
        return()
    endif()

    string(JSON TARGETS_COUNT LENGTH "${TARGETS}")
    if(TARGETS_COUNT EQUAL 0)
        message(STATUS "[publisher] No publish targets configured")
        return()
    endif()
    math(EXPR TARGETS_COUNT "${TARGETS_COUNT} - 1")

    foreach(INDEX RANGE ${TARGETS_COUNT})
        string(JSON TARGET GET "${TARGETS}" ${INDEX})

        # 获取目标类型
        string(JSON TARGET_TYPE GET "${TARGET}" "type")
        if(NOT TARGET_TYPE)
            message(WARNING "[publisher] Target has no type, skipping")
            continue()
        endif()

        if(TARGET_TYPE STREQUAL "http" OR TARGET_TYPE STREQUAL "https")
            # HTTP/HTTPS 发布
            string(JSON TARGET_URL GET "${TARGET}" "url")
            if(NOT TARGET_URL)
                message(WARNING "[publisher] HTTP target has no URL, skipping")
                continue()
            endif()

            # 获取认证信息
            string(JSON PUBLISH_TOKEN GET "${TARGET}" "token")
            string(JSON PUBLISH_USERNAME GET "${TARGET}" "username")
            string(JSON PUBLISH_PASSWORD GET "${TARGET}" "password")

            # 发布每个文件
            foreach(FILE ${FILES_TO_PUBLISH})
                get_filename_component(FILENAME "${FILE}" NAME)
                set(FILE_URL "${TARGET_URL}/${FILENAME}")

                _publish_via_http(
                    FILE "${FILE}"
                    URL "${FILE_URL}"
                    TOKEN "${PUBLISH_TOKEN}"
                    USERNAME "${PUBLISH_USERNAME}"
                    PASSWORD "${PUBLISH_PASSWORD}"
                )
            endforeach()

        elseif(TARGET_TYPE STREQUAL "scp" OR TARGET_TYPE STREQUAL "sftp")
            # SCP/SFTP 发布
            string(JSON HOST_ERROR ERROR_VARIABLE GET "${TARGET}" "host")
            if(HOST_ERROR)
                message(WARNING "[publisher] SCP target has no host, skipping")
                continue()
            endif()
            string(JSON TARGET_HOST GET "${TARGET}" "host")

            # 获取其他配置
            string(JSON USER_ERROR ERROR_VARIABLE GET "${TARGET}" "user")
            string(JSON PORT_ERROR ERROR_VARIABLE GET "${TARGET}" "port")
            string(JSON PATH_ERROR ERROR_VARIABLE GET "${TARGET}" "path")
            string(JSON KEY_ERROR ERROR_VARIABLE GET "${TARGET}" "key_file")

            set(PUBLISH_USER "")
            set(PUBLISH_PORT "22")
            set(PUBLISH_PATH "")
            set(PUBLISH_KEY_FILE "")

            if(NOT USER_ERROR)
                string(JSON PUBLISH_USER GET "${TARGET}" "user")
            endif()
            if(NOT PORT_ERROR)
                string(JSON PUBLISH_PORT GET "${TARGET}" "port")
            endif()
            if(NOT PATH_ERROR)
                string(JSON PUBLISH_PATH GET "${TARGET}" "path")
            endif()
            if(NOT KEY_ERROR)
                string(JSON PUBLISH_KEY_FILE GET "${TARGET}" "key_file")
                if(NOT IS_ABSOLUTE "${PUBLISH_KEY_FILE}")
                    set(PUBLISH_KEY_FILE "${PLUGIN_HOOK_MODULE_DIR}/${PUBLISH_KEY_FILE}")
                endif()
            endif()

            # 发布每个文件
            foreach(FILE ${FILES_TO_PUBLISH})
                _publish_via_scp(
                    FILE "${FILE}"
                    HOST "${TARGET_HOST}"
                    USER "${PUBLISH_USER}"
                    PORT "${PUBLISH_PORT}"
                    PATH "${PUBLISH_PATH}"
                    KEY_FILE "${PUBLISH_KEY_FILE}"
                )
            endforeach()

        else()
            message(WARNING "[publisher] Unknown target type: ${TARGET_TYPE}")
        endif()
    endforeach()

    message(STATUS "[publisher] post_package hook completed")
endfunction()
