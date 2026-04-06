# Module Dependency Manager - 模块依赖自动管理器
# 提供本地依赖查找、远程依赖拉取、版本校验等功能

include(CMakeParseArguments)

# ==============================================================================
# 本地依赖查找
# ==============================================================================

# 查找本地依赖（支持多种搜索路径）
function(find_local_dependency)
    cmake_parse_arguments(
        FIND_LOCAL_DEP
        ""
        "NAME;VERSION;RESULT_VAR;PATH_VAR"
        "SEARCH_PATHS"
        ${ARGN}
    )

    if(NOT FIND_LOCAL_DEP_NAME)
        message(FATAL_ERROR "find_local_dependency: NAME is required")
    endif()

    set(DEP_NAME "${FIND_LOCAL_DEP_NAME}")
    set(FOUND_PATH "")
    set(FOUND_VERSION "")
    set(IS_FOUND FALSE)

    message(STATUS "Searching for local dependency: ${DEP_NAME}")

    # 默认搜索路径
    set(DEFAULT_SEARCH_PATHS
        "${CMAKE_SOURCE_DIR}/libs"
        "${CMAKE_SOURCE_DIR}/modules"
        "${CMAKE_SOURCE_DIR}/deps"
        "${CMAKE_SOURCE_DIR}/../libs"
        "${CMAKE_SOURCE_DIR}/../modules"
    )

    # 合并用户提供的搜索路径和默认路径
    set(ALL_SEARCH_PATHS ${FIND_LOCAL_DEP_SEARCH_PATHS} ${DEFAULT_SEARCH_PATHS})

    # 搜索每个路径
    foreach(SEARCH_PATH ${ALL_SEARCH_PATHS})
        if(NOT EXISTS "${SEARCH_PATH}")
            continue()
        endif()

        # 查找依赖目录
        set(DEP_CANDIDATES "")
        file(GLOB DEP_CANDIDATES "${SEARCH_PATH}/${DEP_NAME}*")

        foreach(CANDIDATE ${DEP_CANDIDATES})
            if(IS_DIRECTORY "${CANDIDATE}")
                # 检查是否有 module.json
                if(EXISTS "${CANDIDATE}/module.json")
                    file(READ "${CANDIDATE}/module.json" MODULE_JSON)
                    string(JSON MODULE_NAME_ERROR ERROR_VARIABLE GET "${MODULE_JSON}" "module" "name")
                    if(NOT MODULE_NAME_ERROR)
                        string(JSON CANDIDATE_NAME GET "${MODULE_JSON}" "module" "name")
                        if("${CANDIDATE_NAME}" STREQUAL "${DEP_NAME}")
                            # 检查版本
                            if(FIND_LOCAL_DEP_VERSION)
                                string(JSON CANDIDATE_VERSION_ERROR ERROR_VARIABLE GET "${MODULE_JSON}" "module" "version")
                                if(NOT CANDIDATE_VERSION_ERROR)
                                    string(JSON CANDIDATE_VERSION GET "${MODULE_JSON}" "module" "version")
                                    if(version_compatible("${CANDIDATE_VERSION}" "${FIND_LOCAL_DEP_VERSION}"))
                                        set(FOUND_PATH "${CANDIDATE}")
                                        set(FOUND_VERSION "${CANDIDATE_VERSION}")
                                        set(IS_FOUND TRUE)
                                        break()
                                    endif()
                                endif()
                            else()
                                # 不要求版本，直接使用
                                string(JSON CANDIDATE_VERSION_ERROR ERROR_VARIABLE GET "${MODULE_JSON}" "module" "version")
                                if(NOT CANDIDATE_VERSION_ERROR)
                                    string(JSON CANDIDATE_VERSION GET "${MODULE_JSON}" "module" "version")
                                    set(FOUND_VERSION "${CANDIDATE_VERSION}")
                                else()
                                    set(FOUND_VERSION "unknown")
                                endif()
                                set(FOUND_PATH "${CANDIDATE}")
                                set(IS_FOUND TRUE)
                                break()
                            endif()
                        endif()
                    endif()
                endif()
            endif()
        endforeach()

        if(IS_FOUND)
            break()
        endif()
    endforeach()

    # 设置结果
    if(FIND_LOCAL_DEP_RESULT_VAR)
        set(${FIND_LOCAL_DEP_RESULT_VAR} ${IS_FOUND} PARENT_SCOPE)
    endif()
    if(FIND_LOCAL_DEP_PATH_VAR)
        set(${FIND_LOCAL_DEP_PATH_VAR} "${FOUND_PATH}" PARENT_SCOPE)
    endif()

    if(IS_FOUND)
        message(STATUS "Found local dependency: ${DEP_NAME} ${FOUND_VERSION} at ${FOUND_PATH}")
    else()
        message(STATUS "Local dependency not found: ${DEP_NAME}")
    endif()
endfunction()

# ==============================================================================
# 版本兼容性检查
# ==============================================================================

# 比较版本号（简单的语义化版本比较）
function(version_compare VERSION1 VERSION2 RESULT_VAR)
    # 拆分版本号
    string(REPLACE "." ";" V1_PARTS "${VERSION1}")
    string(REPLACE "." ";" V2_PARTS "${VERSION2}")

    list(LENGTH V1_PARTS V1_LEN)
    list(LENGTH V2_PARTS V2_LEN)

    set(MAX_LEN ${V1_LEN})
    if(V2_LEN GREATER MAX_LEN)
        set(MAX_LEN ${V2_LEN})
    endif()

    set(RESULT 0)

    foreach(I RANGE 0 ${MAX_LEN})
        if(I LESS V1_LEN)
            list(GET V1_PARTS ${I} V1_PART)
        else()
            set(V1_PART 0)
        endif()

        if(I LESS V2_LEN)
            list(GET V2_PARTS ${I} V2_PART)
        else()
            set(V2_PART 0)
        endif()

        if(V1_PART LESS V2_PART)
            set(RESULT -1)
            break()
        elseif(V1_PART GREATER V2_PART)
            set(RESULT 1)
            break()
        endif()
    endforeach()

    set(${RESULT_VAR} ${RESULT} PARENT_SCOPE)
endfunction()

# 检查版本兼容性（满足最低版本要求）
function(version_compatible ACTUAL_VERSION REQUIRED_VERSION RESULT_VAR)
    if(NOT REQUIRED_VERSION)
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
        return()
    endif()

    version_compare("${ACTUAL_VERSION}" "${REQUIRED_VERSION}" COMPARE_RESULT)
    if(COMPARE_RESULT GREATER_EQUAL 0)
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# ==============================================================================
# Git 依赖拉取
# ==============================================================================

# 从 Git 仓库拉取依赖
function(fetch_git_dependency)
    cmake_parse_arguments(
        FETCH_GIT
        ""
        "NAME;URL;TAG;BRANCH;COMMIT;DEST_DIR;RESULT_VAR"
        ""
        ${ARGN}
    )

    if(NOT FETCH_GIT_NAME OR NOT FETCH_GIT_URL)
        message(FATAL_ERROR "fetch_git_dependency: NAME and URL are required")
    endif()

    if(NOT FETCH_GIT_DEST_DIR)
        set(FETCH_GIT_DEST_DIR "${CMAKE_BINARY_DIR}/deps/${FETCH_GIT_NAME}")
    endif()

    set(DEP_NAME "${FETCH_GIT_NAME}")
    set(DEP_URL "${FETCH_GIT_URL}")
    set(DEP_DEST "${FETCH_GIT_DEST_DIR}")

    message(STATUS "Fetching Git dependency: ${DEP_NAME}")
    message(STATUS "  URL: ${DEP_URL}")
    message(STATUS "  Destination: ${DEP_DEST}")

    # 检查是否已存在
    if(EXISTS "${DEP_DEST}/.git")
        message(STATUS "Git dependency already exists: ${DEP_NAME}")
        # 检查是否需要更新
        if(FETCH_GIT_TAG OR FETCH_GIT_BRANCH OR FETCH_GIT_COMMIT)
            message(STATUS "Checking out specific version...")
            execute_process(
                COMMAND ${GIT_EXECUTABLE} checkout ${FETCH_GIT_TAG}${FETCH_GIT_BRANCH}${FETCH_GIT_COMMIT}
                WORKING_DIRECTORY "${DEP_DEST}"
                RESULT_VARIABLE GIT_CHECKOUT_RESULT
                OUTPUT_QUIET
                ERROR_QUIET
            )
            if(NOT GIT_CHECKOUT_RESULT EQUAL 0)
                message(WARNING "Failed to checkout specific version for ${DEP_NAME}")
            endif()
        endif()
        if(FETCH_GIT_RESULT_VAR)
            set(${FETCH_GIT_RESULT_VAR} TRUE PARENT_SCOPE)
        endif()
        return()
    endif()

    # 查找 Git
    find_package(Git QUIET)
    if(NOT GIT_FOUND)
        message(WARNING "Git not found, cannot fetch dependency: ${DEP_NAME}")
        if(FETCH_GIT_RESULT_VAR)
            set(${FETCH_GIT_RESULT_VAR} FALSE PARENT_SCOPE)
        endif()
        return()
    endif()

    # 克隆仓库
    file(MAKE_DIRECTORY "${DEP_DEST}")

    set(CLONE_ARGS "clone" "${DEP_URL}" "${DEP_DEST}")
    if(FETCH_GIT_BRANCH)
        list(APPEND CLONE_ARGS "--branch" "${FETCH_GIT_BRANCH}")
    endif()

    message(STATUS "Cloning Git repository...")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} ${CLONE_ARGS}
        RESULT_VARIABLE GIT_CLONE_RESULT
        OUTPUT_VARIABLE GIT_CLONE_OUTPUT
        ERROR_VARIABLE GIT_CLONE_ERROR
    )

    if(NOT GIT_CLONE_RESULT EQUAL 0)
        message(WARNING "Failed to clone Git repository for ${DEP_NAME}")
        message(WARNING "Error: ${GIT_CLONE_ERROR}")
        if(FETCH_GIT_RESULT_VAR)
            set(${FETCH_GIT_RESULT_VAR} FALSE PARENT_SCOPE)
        endif()
        return()
    endif()

    # 检出特定标签或提交
    if(FETCH_GIT_TAG)
        message(STATUS "Checking out tag: ${FETCH_GIT_TAG}")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} checkout "tags/${FETCH_GIT_TAG}"
            WORKING_DIRECTORY "${DEP_DEST}"
            RESULT_VARIABLE GIT_CHECKOUT_RESULT
        )
        if(NOT GIT_CHECKOUT_RESULT EQUAL 0)
            message(WARNING "Failed to checkout tag ${FETCH_GIT_TAG} for ${DEP_NAME}")
        endif()
    elseif(FETCH_GIT_COMMIT)
        message(STATUS "Checking out commit: ${FETCH_GIT_COMMIT}")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} checkout "${FETCH_GIT_COMMIT}"
            WORKING_DIRECTORY "${DEP_DEST}"
            RESULT_VARIABLE GIT_CHECKOUT_RESULT
        )
        if(NOT GIT_CHECKOUT_RESULT EQUAL 0)
            message(WARNING "Failed to checkout commit ${FETCH_GIT_COMMIT} for ${DEP_NAME}")
        endif()
    endif()

    message(STATUS "Successfully fetched Git dependency: ${DEP_NAME}")
    if(FETCH_GIT_RESULT_VAR)
        set(${FETCH_GIT_RESULT_VAR} TRUE PARENT_SCOPE)
    endif()
endfunction()

# ==============================================================================
# HTTP 依赖拉取
# ==============================================================================

# 从 HTTP/HTTPS 下载依赖（支持 ZIP/TAR.GZ）
function(fetch_http_dependency)
    cmake_parse_arguments(
        FETCH_HTTP
        ""
        "NAME;URL;DEST_DIR;RESULT_VAR"
        ""
        ${ARGN}
    )

    if(NOT FETCH_HTTP_NAME OR NOT FETCH_HTTP_URL)
        message(FATAL_ERROR "fetch_http_dependency: NAME and URL are required")
    endif()

    if(NOT FETCH_HTTP_DEST_DIR)
        set(FETCH_HTTP_DEST_DIR "${CMAKE_BINARY_DIR}/deps/${FETCH_HTTP_NAME}")
    endif()

    set(DEP_NAME "${FETCH_HTTP_NAME}")
    set(DEP_URL "${FETCH_HTTP_URL}")
    set(DEP_DEST "${FETCH_HTTP_DEST_DIR}")
    set(ARCHIVE_PATH "${CMAKE_BINARY_DIR}/deps/${DEP_NAME}.archive")

    message(STATUS "Fetching HTTP dependency: ${DEP_NAME}")
    message(STATUS "  URL: ${DEP_URL}")

    # 检查是否已存在
    if(EXISTS "${DEP_DEST}")
        file(GLOB DEP_CONTENTS "${DEP_DEST}/*")
        if(DEP_CONTENTS)
            message(STATUS "HTTP dependency already exists: ${DEP_NAME}")
            if(FETCH_HTTP_RESULT_VAR)
                set(${FETCH_HTTP_RESULT_VAR} TRUE PARENT_SCOPE)
            endif()
            return()
        endif()
    endif()

    # 确保目录存在
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/deps")
    file(MAKE_DIRECTORY "${DEP_DEST}")

    # 下载文件
    message(STATUS "Downloading file...")
    file(DOWNLOAD
        "${DEP_URL}"
        "${ARCHIVE_PATH}"
        STATUS DOWNLOAD_STATUS
        SHOW_PROGRESS
    )

    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_STATUS_CODE)
    if(NOT DOWNLOAD_STATUS_CODE EQUAL 0)
        list(GET DOWNLOAD_STATUS 1 DOWNLOAD_ERROR)
        message(WARNING "Failed to download ${DEP_NAME}: ${DOWNLOAD_ERROR}")
        file(REMOVE "${ARCHIVE_PATH}")
        if(FETCH_HTTP_RESULT_VAR)
            set(${FETCH_HTTP_RESULT_VAR} FALSE PARENT_SCOPE)
        endif()
        return()
    endif()

    # 解压文件
    message(STATUS "Extracting archive...")
    if("${DEP_URL}" MATCHES "\\.zip$")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${ARCHIVE_PATH}"
            WORKING_DIRECTORY "${DEP_DEST}"
            RESULT_VARIABLE EXTRACT_RESULT
        )
    elseif("${DEP_URL}" MATCHES "\\.(tar\\.gz|tgz)$")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${ARCHIVE_PATH}"
            WORKING_DIRECTORY "${DEP_DEST}"
            RESULT_VARIABLE EXTRACT_RESULT
        )
    else()
        message(WARNING "Unknown archive format for ${DEP_URL}")
        file(REMOVE "${ARCHIVE_PATH}")
        if(FETCH_HTTP_RESULT_VAR)
            set(${FETCH_HTTP_RESULT_VAR} FALSE PARENT_SCOPE)
        endif()
        return()
    endif()

    if(NOT EXTRACT_RESULT EQUAL 0)
        message(WARNING "Failed to extract archive for ${DEP_NAME}")
        file(REMOVE "${ARCHIVE_PATH}")
        if(FETCH_HTTP_RESULT_VAR)
            set(${FETCH_HTTP_RESULT_VAR} FALSE PARENT_SCOPE)
        endif()
        return()
    endif()

    # 清理临时文件
    file(REMOVE "${ARCHIVE_PATH}")

    message(STATUS "Successfully fetched HTTP dependency: ${DEP_NAME}")
    if(FETCH_HTTP_RESULT_VAR)
        set(${FETCH_HTTP_RESULT_VAR} TRUE PARENT_SCOPE)
    endif()
endfunction()

# ==============================================================================
# 综合依赖管理
# ==============================================================================

# 解析并处理 module.json 中的依赖配置
function(process_module_dependencies)
    cmake_parse_arguments(
        PROCESS_DEPS
        ""
        "MODULE_DIR;NAMESPACE"
        ""
        ${ARGN}
    )

    if(NOT PROCESS_DEPS_MODULE_DIR)
        set(PROCESS_DEPS_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    set(MODULE_JSON_PATH "${PROCESS_DEPS_MODULE_DIR}/module.json")
    if(NOT EXISTS "${MODULE_JSON_PATH}")
        message(WARNING "module.json not found at ${MODULE_JSON_PATH}")
        return()
    endif()

    file(READ "${MODULE_JSON_PATH}" MODULE_JSON_CONTENT)

    # 检查是否有 dependencies 配置
    string(JSON DEPS_CONFIG GET "${MODULE_JSON_CONTENT}" "dependencies")
    if(NOT DEPS_CONFIG)
        # 没有 dependencies 配置，直接返回
        return()
    endif()
    string(JSON DEPS_COUNT LENGTH "${DEPS_CONFIG}")

    if(DEPS_COUNT EQUAL 0)
        return()
    endif()

    message(STATUS "Processing ${DEPS_COUNT} dependencies...")

    math(EXPR DEPS_COUNT "${DEPS_COUNT} - 1")
    foreach(INDEX RANGE ${DEPS_COUNT})
        string(JSON DEP_CONFIG GET "${DEPS_CONFIG}" ${INDEX})

        # 提取依赖基本信息
        string(JSON DEP_NAME GET "${DEP_CONFIG}" "name")
        string(JSON DEP_VERSION_ERROR ERROR_VARIABLE GET "${DEP_CONFIG}" "version")
        if(DEP_VERSION_ERROR)
            set(DEP_VERSION "")
        else()
            string(JSON DEP_VERSION GET "${DEP_CONFIG}" "version")
        endif()

        message(STATUS "Processing dependency: ${DEP_NAME} ${DEP_VERSION}")

        # 1. 先尝试查找本地依赖
        set(DEP_FOUND FALSE)
        set(DEP_PATH "")

        find_local_dependency(
            NAME "${DEP_NAME}"
            VERSION "${DEP_VERSION}"
            RESULT_VAR DEP_FOUND
            PATH_VAR DEP_PATH
        )

        # 2. 如果本地没有，尝试从远程拉取
        if(NOT DEP_FOUND)
            string(JSON DEP_SOURCE_ERROR ERROR_VARIABLE GET "${DEP_CONFIG}" "source")
            if(NOT DEP_SOURCE_ERROR)
                string(JSON DEP_SOURCE GET "${DEP_CONFIG}" "source")
                string(JSON SOURCE_TYPE GET "${DEP_SOURCE}" "type")

                if("${SOURCE_TYPE}" STREQUAL "git")
                    string(JSON SOURCE_URL GET "${DEP_SOURCE}" "url")
                    string(JSON SOURCE_TAG_ERROR ERROR_VARIABLE GET "${DEP_SOURCE}" "tag")
                    string(JSON SOURCE_BRANCH_ERROR ERROR_VARIABLE GET "${DEP_SOURCE}" "branch")
                    string(JSON SOURCE_COMMIT_ERROR ERROR_VARIABLE GET "${DEP_SOURCE}" "commit")

                    set(GIT_ARGS "NAME" "${DEP_NAME}" "URL" "${SOURCE_URL}")
                    if(NOT SOURCE_TAG_ERROR)
                        string(JSON SOURCE_TAG GET "${DEP_SOURCE}" "tag")
                        list(APPEND GIT_ARGS "TAG" "${SOURCE_TAG}")
                    endif()
                    if(NOT SOURCE_BRANCH_ERROR)
                        string(JSON SOURCE_BRANCH GET "${DEP_SOURCE}" "branch")
                        list(APPEND GIT_ARGS "BRANCH" "${SOURCE_BRANCH}")
                    endif()
                    if(NOT SOURCE_COMMIT_ERROR)
                        string(JSON SOURCE_COMMIT GET "${DEP_SOURCE}" "commit")
                        list(APPEND GIT_ARGS "COMMIT" "${SOURCE_COMMIT}")
                    endif()

                    fetch_git_dependency(${GIT_ARGS} RESULT_VAR DEP_FETCHED)
                    if(DEP_FETCHED)
                        set(DEP_FOUND TRUE)
                        set(DEP_PATH "${CMAKE_BINARY_DIR}/deps/${DEP_NAME}")
                    endif()

                elseif("${SOURCE_TYPE}" STREQUAL "http")
                    string(JSON SOURCE_URL GET "${DEP_SOURCE}" "url")
                    fetch_http_dependency(
                        NAME "${DEP_NAME}"
                        URL "${SOURCE_URL}"
                        RESULT_VAR DEP_FETCHED
                    )
                    if(DEP_FETCHED)
                        set(DEP_FOUND TRUE)
                        set(DEP_PATH "${CMAKE_BINARY_DIR}/deps/${DEP_NAME}")
                    endif()
                endif()
            endif()
        endif()

        # 3. 如果找到依赖，添加到构建
        if(DEP_FOUND AND EXISTS "${DEP_PATH}/CMakeLists.txt")
            message(STATUS "Adding dependency to build: ${DEP_NAME}")
            add_subdirectory("${DEP_PATH}" "${CMAKE_BINARY_DIR}/deps-build/${DEP_NAME}")
        elseif(DEP_FOUND)
            message(STATUS "Dependency found but no CMakeLists.txt: ${DEP_NAME}")
        else()
            message(WARNING "Dependency not found: ${DEP_NAME}")
        endif()
    endforeach()
endfunction()
