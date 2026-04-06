# Signer Plugin - 模块签名插件
# 为构建产物生成数字签名

include(CMakeParseArguments)

# 注册插件
register_plugin(
    NAME "signer"
    DESCRIPTION "Sign built modules with digital signatures"
    VERSION "1.0.0"
    AUTHOR "IDCU Team"
    HOOKS "post_package"
    DEPENDS ""
)

# 查找签名工具
function(_find_sign_tool RESULT_VAR)
    # 查找 OpenSSL
    find_program(OPENSSL_EXECUTABLE openssl)
    if(OPENSSL_EXECUTABLE)
        set(${RESULT_VAR} "openssl" PARENT_SCOPE)
        return()
    endif()

    # 查找 GPG
    find_program(GPG_EXECUTABLE gpg)
    if(GPG_EXECUTABLE)
        set(${RESULT_VAR} "gpg" PARENT_SCOPE)
        return()
    endif()

    # 查找 signtool (Windows)
    if(WIN32)
        find_program(SIGNTOOL_EXECUTABLE signtool)
        if(SIGNTOOL_EXECUTABLE)
            set(${RESULT_VAR} "signtool" PARENT_SCOPE)
            return()
        endif()
    endif()

    set(${RESULT_VAR} "" PARENT_SCOPE)
endfunction()

# 使用 OpenSSL 签名
function(_sign_with_openssl)
    cmake_parse_arguments(
        SIGN_OPENSSL
        ""
        "INPUT_FILE;OUTPUT_DIR;KEY_FILE;CERT_FILE"
        ""
        ${ARGN}
    )

    if(NOT SIGN_OPENSSL_INPUT_FILE)
        message(WARNING "[signer] No input file for OpenSSL signing")
        return()
    endif()

    if(NOT SIGN_OPENSSL_OUTPUT_DIR)
        get_filename_component(SIGN_OPENSSL_OUTPUT_DIR "${SIGN_OPENSSL_INPUT_FILE}" DIRECTORY)
    endif()

    find_program(OPENSSL_EXECUTABLE openssl)
    if(NOT OPENSSL_EXECUTABLE)
        message(WARNING "[signer] openssl not found, skipping signing")
        return()
    endif()

    get_filename_component(INPUT_FILENAME "${SIGN_OPENSSL_INPUT_FILE}" NAME)
    set(SIGNATURE_FILE "${SIGN_OPENSSL_OUTPUT_DIR}/${INPUT_FILENAME}.sig")

    # 如果没有提供密钥，生成临时密钥对用于演示
    if(NOT SIGN_OPENSSL_KEY_FILE OR NOT EXISTS "${SIGN_OPENSSL_KEY_FILE}")
        message(STATUS "[signer] Generating temporary key pair for signing")

        set(TEMP_KEY "${SIGN_OPENSSL_OUTPUT_DIR}/temp_key.pem")
        set(TEMP_CERT "${SIGN_OPENSSL_OUTPUT_DIR}/temp_cert.pem")

        # 生成私钥
        execute_process(
            COMMAND ${OPENSSL_EXECUTABLE} genrsa -out "${TEMP_KEY}" 2048
            OUTPUT_QUIET
            ERROR_QUIET
        )

        # 生成自签名证书
        execute_process(
            COMMAND ${OPENSSL_EXECUTABLE} req -new -x509 -key "${TEMP_KEY}" -out "${TEMP_CERT}" -days 365 -subj "/CN=IDCU Module Signer"
            OUTPUT_QUIET
            ERROR_QUIET
        )

        set(SIGN_OPENSSL_KEY_FILE "${TEMP_KEY}")
        set(SIGN_OPENSSL_CERT_FILE "${TEMP_CERT}")
    endif()

    # 使用 SHA256 签名
    message(STATUS "[signer] Signing ${INPUT_FILENAME} with OpenSSL")

    execute_process(
        COMMAND ${OPENSSL_EXECUTABLE} dgst -sha256 -sign "${SIGN_OPENSSL_KEY_FILE}" -out "${SIGNATURE_FILE}" "${SIGN_OPENSSL_INPUT_FILE}"
        RESULT_VARIABLE SIGN_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )

    if(SIGN_RESULT EQUAL 0)
        message(STATUS "[signer] Signature created: ${SIGNATURE_FILE}")

        # 生成公钥用于验证
        if(SIGN_OPENSSL_CERT_FILE)
            configure_file("${SIGN_OPENSSL_CERT_FILE}" "${SIGN_OPENSSL_OUTPUT_DIR}/signing_cert.pem" COPYONLY)
            message(STATUS "[signer] Certificate copied: ${SIGN_OPENSSL_OUTPUT_DIR}/signing_cert.pem")
        endif()
    else()
        message(WARNING "[signer] Failed to sign file")
    endif()
endfunction()

# 使用 GPG 签名
function(_sign_with_gpg)
    cmake_parse_arguments(
        SIGN_GPG
        ""
        "INPUT_FILE;OUTPUT_DIR;KEY_ID"
        ""
        ${ARGN}
    )

    if(NOT SIGN_GPG_INPUT_FILE)
        message(WARNING "[signer] No input file for GPG signing")
        return()
    endif()

    if(NOT SIGN_GPG_OUTPUT_DIR)
        get_filename_component(SIGN_GPG_OUTPUT_DIR "${SIGN_GPG_INPUT_FILE}" DIRECTORY)
    endif()

    find_program(GPG_EXECUTABLE gpg)
    if(NOT GPG_EXECUTABLE)
        message(WARNING "[signer] gpg not found, skipping signing")
        return()
    endif()

    get_filename_component(INPUT_FILENAME "${SIGN_GPG_INPUT_FILE}" NAME)
    set(SIGNATURE_FILE "${SIGN_GPG_OUTPUT_DIR}/${INPUT_FILENAME}.asc")

    message(STATUS "[signer] Signing ${INPUT_FILENAME} with GPG")

    set(GPG_ARGS "--armor" "--detach-sign" "--output" "${SIGNATURE_FILE}")
    if(SIGN_GPG_KEY_ID)
        list(APPEND GPG_ARGS "--local-user" "${SIGN_GPG_KEY_ID}")
    endif()
    list(APPEND GPG_ARGS "${SIGN_GPG_INPUT_FILE}")

    execute_process(
        COMMAND ${GPG_EXECUTABLE} ${GPG_ARGS}
        RESULT_VARIABLE SIGN_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )

    if(SIGN_RESULT EQUAL 0)
        message(STATUS "[signer] GPG signature created: ${SIGNATURE_FILE}")
    else()
        message(WARNING "[signer] Failed to sign with GPG")
    endif()
endfunction()

# 使用 signtool 签名 (Windows)
function(_sign_with_signtool)
    cmake_parse_arguments(
        SIGN_SIGNTOOL
        ""
        "INPUT_FILE;OUTPUT_DIR;CERT_FILE;CERT_PASSWORD;TIMESTAMP_URL"
        ""
        ${ARGN}
    )

    if(NOT WIN32)
        return()
    endif()

    if(NOT SIGN_SIGNTOOL_INPUT_FILE)
        message(WARNING "[signer] No input file for signtool")
        return()
    endif()

    find_program(SIGNTOOL_EXECUTABLE signtool)
    if(NOT SIGNTOOL_EXECUTABLE)
        message(WARNING "[signer] signtool not found, skipping signing")
        return()
    endif()

    get_filename_component(INPUT_FILENAME "${SIGN_SIGNTOOL_INPUT_FILE}" NAME)
    message(STATUS "[signer] Signing ${INPUT_FILENAME} with signtool")

    set(SIGNTOOL_ARGS "sign" "/fd" "SHA256")

    if(SIGN_SIGNTOOL_CERT_FILE)
        list(APPEND SIGNTOOL_ARGS "/f" "${SIGN_SIGNTOOL_CERT_FILE}")
        if(SIGN_SIGNTOOL_CERT_PASSWORD)
            list(APPEND SIGNTOOL_ARGS "/p" "${SIGN_SIGNTOOL_CERT_PASSWORD}")
        endif()
    endif()

    if(SIGN_SIGNTOOL_TIMESTAMP_URL)
        list(APPEND SIGNTOOL_ARGS "/tr" "${SIGN_SIGNTOOL_TIMESTAMP_URL}" "/td" "SHA256")
    endif()

    list(APPEND SIGNTOOL_ARGS "${SIGN_SIGNTOOL_INPUT_FILE}")

    execute_process(
        COMMAND ${SIGNTOOL_EXECUTABLE} ${SIGNTOOL_ARGS}
        RESULT_VARIABLE SIGN_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )

    if(SIGN_RESULT EQUAL 0)
        message(STATUS "[signer] signtool signing completed")
    else()
        message(WARNING "[signer] Failed to sign with signtool")
    endif()
endfunction()

# 钩子分发函数
function(signer_dispatch_hook)
    cmake_parse_arguments(
        DISPATCH
        ""
        "HOOK_NAME;MODULE_DIR;MODULE_NAME;MODULE_VERSION"
        ""
        ${ARGN}
    )

    if(DISPATCH_HOOK_NAME STREQUAL "post_package")
        signer_post_package(
            MODULE_DIR "${DISPATCH_MODULE_DIR}"
            MODULE_NAME "${DISPATCH_MODULE_NAME}"
            MODULE_VERSION "${DISPATCH_MODULE_VERSION}"
        )
    endif()
endfunction()

# post_package 钩子实现
function(signer_post_package)
    cmake_parse_arguments(
        PLUGIN_HOOK
        ""
        "MODULE_DIR;MODULE_NAME;MODULE_VERSION"
        ""
        ${ARGN}
    )

    message(STATUS "[signer] Running post_package hook for ${PLUGIN_HOOK_MODULE_NAME}")

    # 查找可用的签名工具
    _find_sign_tool(SIGN_TOOL)

    if(NOT SIGN_TOOL)
        message(WARNING "[signer] No signing tool found (openssl/gpg/signtool), skipping signing")
        return()
    endif()

    message(STATUS "[signer] Using signing tool: ${SIGN_TOOL}")

    # 确定打包输出目录
    set(PACKAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")

    # 查找要签名的文件
    set(SIGN_FILES "")

    # 查找库文件
    file(GLOB_RECURSE LIB_FILES
        "${PACKAGE_OUTPUT_DIR}/*.dll"
        "${PACKAGE_OUTPUT_DIR}/*.so"
        "${PACKAGE_OUTPUT_DIR}/*.dylib"
        "${PACKAGE_OUTPUT_DIR}/*.lib"
        "${PACKAGE_OUTPUT_DIR}/*.a"
    )
    list(APPEND SIGN_FILES ${LIB_FILES})

    # 查找可执行文件
    file(GLOB_RECURSE EXE_FILES
        "${PACKAGE_OUTPUT_DIR}/*.exe"
    )
    list(APPEND SIGN_FILES ${EXE_FILES})

    if(NOT SIGN_FILES)
        message(STATUS "[signer] No files found to sign")
        return()
    endif()

    message(STATUS "[signer] Found files to sign")

    # 尝试从 module.json 读取签名配置
    set(MODULE_JSON_PATH "${PLUGIN_HOOK_MODULE_DIR}/module.json")
    set(SIGN_KEY_FILE "")
    set(SIGN_CERT_FILE "")
    set(SIGN_GPG_KEY_ID "")

    if(EXISTS "${MODULE_JSON_PATH}")
        file(READ "${MODULE_JSON_PATH}" MODULE_JSON_CONTENT)

        # 检查是否有 signer 配置
        string(JSON SIGNER_CONFIG GET "${MODULE_JSON_CONTENT}" "signer")
        if(SIGNER_CONFIG)
            string(JSON SIGN_KEY_FILE GET "${SIGNER_CONFIG}" "key_file")
            if(SIGN_KEY_FILE AND NOT IS_ABSOLUTE "${SIGN_KEY_FILE}")
                set(SIGN_KEY_FILE "${PLUGIN_HOOK_MODULE_DIR}/${SIGN_KEY_FILE}")
            endif()

            string(JSON SIGN_CERT_FILE GET "${SIGNER_CONFIG}" "cert_file")
            if(SIGN_CERT_FILE AND NOT IS_ABSOLUTE "${SIGN_CERT_FILE}")
                set(SIGN_CERT_FILE "${PLUGIN_HOOK_MODULE_DIR}/${SIGN_CERT_FILE}")
            endif()

            string(JSON SIGN_GPG_KEY_ID GET "${SIGNER_CONFIG}" "gpg_key_id")
        endif()
    endif()

    # 对每个文件进行签名
    foreach(FILE_TO_SIGN ${SIGN_FILES})
        get_filename_component(FILE_DIR "${FILE_TO_SIGN}" DIRECTORY)

        if(SIGN_TOOL STREQUAL "openssl")
            _sign_with_openssl(
                INPUT_FILE "${FILE_TO_SIGN}"
                OUTPUT_DIR "${FILE_DIR}"
                KEY_FILE "${SIGN_KEY_FILE}"
                CERT_FILE "${SIGN_CERT_FILE}"
            )
        elseif(SIGN_TOOL STREQUAL "gpg")
            _sign_with_gpg(
                INPUT_FILE "${FILE_TO_SIGN}"
                OUTPUT_DIR "${FILE_DIR}"
                KEY_ID "${SIGN_GPG_KEY_ID}"
            )
        elseif(SIGN_TOOL STREQUAL "signtool")
            _sign_with_signtool(
                INPUT_FILE "${FILE_TO_SIGN}"
                OUTPUT_DIR "${FILE_DIR}"
                CERT_FILE "${SIGN_CERT_FILE}"
            )
        endif()
    endforeach()

    message(STATUS "[signer] post_package hook completed")
endfunction()
