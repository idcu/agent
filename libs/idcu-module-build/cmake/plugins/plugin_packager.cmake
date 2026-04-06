# Packager Plugin - 打包插件
# 将构建产物打包为 ZIP 或 TAR 文件

include(CMakeParseArguments)

# 注册插件
register_plugin(
    NAME "packager"
    DESCRIPTION "Package built modules into ZIP or TAR archives"
    VERSION "1.0.0"
    AUTHOR "IDCU Team"
    HOOKS "post_package"
    DEPENDS ""
)

# 查找压缩工具
function(_find_archive_tools RESULT_VAR)
    set(TOOLS "")

    # 查找 zip
    find_program(ZIP_EXECUTABLE zip)
    if(ZIP_EXECUTABLE)
        list(APPEND TOOLS "zip")
    endif()

    # 查找 unzip
    find_program(UNZIP_EXECUTABLE unzip)
    if(UNZIP_EXECUTABLE)
        list(APPEND TOOLS "unzip")
    endif()

    # 查找 tar
    find_program(TAR_EXECUTABLE tar)
    if(TAR_EXECUTABLE)
        list(APPEND TOOLS "tar")
    endif()

    # 查找 7z
    find_program(Z7_EXECUTABLE 7z)
    if(Z7_EXECUTABLE)
        list(APPEND TOOLS "7z")
    endif()

    # Windows: 使用 PowerShell 的 Compress-Archive
    if(WIN32)
        list(APPEND TOOLS "powershell")
    endif()

    set(${RESULT_VAR} "${TOOLS}" PARENT_SCOPE)
endfunction()

# 使用 zip 创建压缩包
function(_create_zip_archive)
    cmake_parse_arguments(
        CREATE_ZIP
        ""
        "SOURCE_DIR;OUTPUT_FILE"
        ""
        ${ARGN}
    )

    if(NOT CREATE_ZIP_SOURCE_DIR)
        message(WARNING "[packager] No source directory for zip")
        return()
    endif()

    if(NOT CREATE_ZIP_OUTPUT_FILE)
        get_filename_component(SOURCE_NAME "${CREATE_ZIP_SOURCE_DIR}" NAME)
        set(CREATE_ZIP_OUTPUT_FILE "${CMAKE_BINARY_DIR}/${SOURCE_NAME}.zip")
    endif()

    # 确保输出目录存在
    get_filename_component(OUTPUT_DIR "${CREATE_ZIP_OUTPUT_FILE}" DIRECTORY)
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")

    message(STATUS "[packager] Creating ZIP archive: ${CREATE_ZIP_OUTPUT_FILE}")

    # 查找可用工具
    find_program(ZIP_EXECUTABLE zip)
    find_program(Z7_EXECUTABLE 7z)

    if(ZIP_EXECUTABLE)
        # 使用 zip 命令
        get_filename_component(SOURCE_PARENT "${CREATE_ZIP_SOURCE_DIR}" DIRECTORY)
        get_filename_component(SOURCE_NAME "${CREATE_ZIP_SOURCE_DIR}" NAME)

        execute_process(
            COMMAND ${ZIP_EXECUTABLE} -r "${CREATE_ZIP_OUTPUT_FILE}" "${SOURCE_NAME}"
            WORKING_DIRECTORY "${SOURCE_PARENT}"
            RESULT_VARIABLE ZIP_RESULT
            OUTPUT_QUIET
            ERROR_QUIET
        )

        if(ZIP_RESULT EQUAL 0)
            message(STATUS "[packager] ZIP archive created successfully")
        else()
            message(WARNING "[packager] Failed to create ZIP with zip command")
        endif()
    elseif(Z7_EXECUTABLE)
        # 使用 7z
        execute_process(
            COMMAND ${Z7_EXECUTABLE} a -tzip "${CREATE_ZIP_OUTPUT_FILE}" "${CREATE_ZIP_SOURCE_DIR}/*"
            RESULT_VARIABLE Z7_RESULT
            OUTPUT_QUIET
            ERROR_QUIET
        )

        if(Z7_RESULT EQUAL 0)
            message(STATUS "[packager] ZIP archive created successfully with 7z")
        else()
            message(WARNING "[packager] Failed to create ZIP with 7z")
        endif()
    elseif(WIN32)
        # 使用 PowerShell
        execute_process(
            COMMAND powershell -Command "Compress-Archive -Path '${CREATE_ZIP_SOURCE_DIR}/*' -DestinationPath '${CREATE_ZIP_OUTPUT_FILE}' -Force"
            RESULT_VARIABLE PS_RESULT
            OUTPUT_QUIET
            ERROR_QUIET
        )

        if(PS_RESULT EQUAL 0)
            message(STATUS "[packager] ZIP archive created successfully with PowerShell")
        else()
            message(WARNING "[packager] Failed to create ZIP with PowerShell")
        endif()
    else()
        message(WARNING "[packager] No ZIP tool available")
    endif()
endfunction()

# 使用 tar 创建压缩包
function(_create_tar_archive)
    cmake_parse_arguments(
        CREATE_TAR
        ""
        "SOURCE_DIR;OUTPUT_FILE;COMPRESSION"
        ""
        ${ARGN}
    )

    if(NOT CREATE_TAR_SOURCE_DIR)
        message(WARNING "[packager] No source directory for tar")
        return()
    endif()

    if(NOT CREATE_TAR_OUTPUT_FILE)
        get_filename_component(SOURCE_NAME "${CREATE_TAR_SOURCE_DIR}" NAME)
        set(CREATE_TAR_OUTPUT_FILE "${CMAKE_BINARY_DIR}/${SOURCE_NAME}.tar.gz")
    endif()

    if(NOT CREATE_TAR_COMPRESSION)
        # 根据文件扩展名推断压缩方式
        if(CREATE_TAR_OUTPUT_FILE MATCHES "\\.tar\\.gz$")
            set(CREATE_TAR_COMPRESSION "gzip")
        elseif(CREATE_TAR_OUTPUT_FILE MATCHES "\\.tar\\.bz2$")
            set(CREATE_TAR_COMPRESSION "bzip2")
        elseif(CREATE_TAR_OUTPUT_FILE MATCHES "\\.tar\\.xz$")
            set(CREATE_TAR_COMPRESSION "xz")
        else()
            set(CREATE_TAR_COMPRESSION "gzip")
        endif()
    endif()

    # 确保输出目录存在
    get_filename_component(OUTPUT_DIR "${CREATE_TAR_OUTPUT_FILE}" DIRECTORY)
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")

    message(STATUS "[packager] Creating TAR archive: ${CREATE_TAR_OUTPUT_FILE}")

    find_program(TAR_EXECUTABLE tar)
    if(NOT TAR_EXECUTABLE)
        message(WARNING "[packager] tar not found")
        return()
    endif()

    # 构建 tar 参数
    set(TAR_ARGS "cf")

    if(CREATE_TAR_COMPRESSION STREQUAL "gzip")
        list(APPEND TAR_ARGS "z")
    elseif(CREATE_TAR_COMPRESSION STREQUAL "bzip2")
        list(APPEND TAR_ARGS "j")
    elseif(CREATE_TAR_COMPRESSION STREQUAL "xz")
        list(APPEND TAR_ARGS "J")
    endif()

    list(APPEND TAR_ARGS "${CREATE_TAR_OUTPUT_FILE}")

    # 添加源文件
    get_filename_component(SOURCE_PARENT "${CREATE_TAR_SOURCE_DIR}" DIRECTORY)
    get_filename_component(SOURCE_NAME "${CREATE_TAR_SOURCE_DIR}" NAME)
    list(APPEND TAR_ARGS "${SOURCE_NAME}")

    execute_process(
        COMMAND ${TAR_EXECUTABLE} ${TAR_ARGS}
        WORKING_DIRECTORY "${SOURCE_PARENT}"
        RESULT_VARIABLE TAR_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )

    if(TAR_RESULT EQUAL 0)
        message(STATUS "[packager] TAR archive created successfully")
    else()
        message(WARNING "[packager] Failed to create TAR archive")
    endif()
endfunction()

# 钩子分发函数
function(packager_dispatch_hook)
    cmake_parse_arguments(
        DISPATCH
        ""
        "HOOK_NAME;MODULE_DIR;MODULE_NAME;MODULE_VERSION"
        ""
        ${ARGN}
    )

    if(DISPATCH_HOOK_NAME STREQUAL "post_package")
        packager_post_package(
            MODULE_DIR "${DISPATCH_MODULE_DIR}"
            MODULE_NAME "${DISPATCH_MODULE_NAME}"
            MODULE_VERSION "${DISPATCH_MODULE_VERSION}"
        )
    endif()
endfunction()

# post_package 钩子实现
function(packager_post_package)
    cmake_parse_arguments(
        PLUGIN_HOOK
        ""
        "MODULE_DIR;MODULE_NAME;MODULE_VERSION"
        ""
        ${ARGN}
    )

    message(STATUS "[packager] Running post_package hook for ${PLUGIN_HOOK_MODULE_NAME}")

    # 确定打包输出目录
    set(PACKAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")

    # 查找要打包的目录
    set(SOURCE_DIR "${PACKAGE_OUTPUT_DIR}")

    # 查找格式为 {module}-{version}-{platform}-{arch} 的目录
    file(GLOB PACKAGE_DIRS "${PACKAGE_OUTPUT_DIR}/${PLUGIN_HOOK_MODULE_NAME}-*")
    if(PACKAGE_DIRS)
        list(GET PACKAGE_DIRS 0 SOURCE_DIR)
    endif()

    if(NOT EXISTS "${SOURCE_DIR}")
        message(WARNING "[packager] Source directory not found: ${SOURCE_DIR}")
        return()
    endif()

    get_filename_component(PACKAGE_NAME "${SOURCE_DIR}" NAME)

    # 从 module.json 读取打包配置
    set(MODULE_JSON_PATH "${PLUGIN_HOOK_MODULE_DIR}/module.json")
    set(FORMATS "zip")
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}")

    if(EXISTS "${MODULE_JSON_PATH}")
        file(READ "${MODULE_JSON_PATH}" MODULE_JSON_CONTENT)

        string(JSON PACKAGER_CONFIG GET "${MODULE_JSON_CONTENT}" "packager")
        if(PACKAGER_CONFIG)
            # 读取格式配置
            string(JSON FORMATS_LIST GET "${PACKAGER_CONFIG}" "formats")
            if(FORMATS_LIST)
                string(JSON FORMATS_COUNT LENGTH "${FORMATS_LIST}")
                if(FORMATS_COUNT GREATER 0)
                    set(FORMATS "")
                    math(EXPR FORMATS_COUNT "${FORMATS_COUNT} - 1")
                    foreach(INDEX RANGE ${FORMATS_COUNT})
                        string(JSON FORMAT GET "${FORMATS_LIST}" ${INDEX})
                        list(APPEND FORMATS "${FORMAT}")
                    endforeach()
                endif()
            endif()

            # 读取输出目录
            string(JSON OUTPUT_DIR_VALUE GET "${PACKAGER_CONFIG}" "output_dir")
            if(OUTPUT_DIR_VALUE)
                if(NOT IS_ABSOLUTE "${OUTPUT_DIR_VALUE}")
                    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/${OUTPUT_DIR_VALUE}")
                else()
                    set(OUTPUT_DIR "${OUTPUT_DIR_VALUE}")
                endif()
            endif()
        endif()
    endif()

    message(STATUS "[packager] Package formats: ${FORMATS}")
    message(STATUS "[packager] Output directory: ${OUTPUT_DIR}")

    # 查找可用工具
    _find_archive_tools(ARCHIVE_TOOLS)
    message(STATUS "[packager] Available tools: ${ARCHIVE_TOOLS}")

    # 创建压缩包
    foreach(FORMAT ${FORMATS})
        if(FORMAT STREQUAL "zip")
            _create_zip_archive(
                SOURCE_DIR "${SOURCE_DIR}"
                OUTPUT_FILE "${OUTPUT_DIR}/${PACKAGE_NAME}.zip"
            )
        elseif(FORMAT STREQUAL "tar.gz")
            _create_tar_archive(
                SOURCE_DIR "${SOURCE_DIR}"
                OUTPUT_FILE "${OUTPUT_DIR}/${PACKAGE_NAME}.tar.gz"
                COMPRESSION "gzip"
            )
        elseif(FORMAT STREQUAL "tar.bz2")
            _create_tar_archive(
                SOURCE_DIR "${SOURCE_DIR}"
                OUTPUT_FILE "${OUTPUT_DIR}/${PACKAGE_NAME}.tar.bz2"
                COMPRESSION "bzip2"
            )
        elseif(FORMAT STREQUAL "tar.xz")
            _create_tar_archive(
                SOURCE_DIR "${SOURCE_DIR}"
                OUTPUT_FILE "${OUTPUT_DIR}/${PACKAGE_NAME}.tar.xz"
                COMPRESSION "xz"
            )
        else()
            message(WARNING "[packager] Unknown format: ${FORMAT}")
        endif()
    endforeach()

    message(STATUS "[packager] post_package hook completed")
endfunction()
