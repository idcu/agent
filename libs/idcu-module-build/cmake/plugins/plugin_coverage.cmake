# Coverage Plugin - 代码覆盖率插件
# 为测试生成代码覆盖率报告

include(CMakeParseArguments)

# 注册插件
register_plugin(
    NAME "coverage"
    DESCRIPTION "Generate code coverage reports for tests"
    VERSION "1.0.0"
    AUTHOR "IDCU Team"
    HOOKS "post_build"
    DEPENDS ""
)

# 查找覆盖率工具
function(_find_coverage_tools RESULT_VAR)
    set(TOOLS "")

    # 查找 gcov/lcov (GCC/Clang)
    find_program(GCOV_EXECUTABLE gcov)
    if(GCOV_EXECUTABLE)
        list(APPEND TOOLS "gcov")
    endif()

    find_program(LCOV_EXECUTABLE lcov)
    if(LCOV_EXECUTABLE)
        list(APPEND TOOLS "lcov")
    endif()

    find_program(GENHTML_EXECUTABLE genhtml)
    if(GENHTML_EXECUTABLE)
        list(APPEND TOOLS "genhtml")
    endif()

    # Windows: 查找 OpenCppCoverage
    if(WIN32)
        find_program(OPENCPPCOVERAGE_EXECUTABLE OpenCppCoverage)
        if(OPENCPPCOVERAGE_EXECUTABLE)
            list(APPEND TOOLS "opencppcoverage")
        endif()
    endif()

    set(${RESULT_VAR} "${TOOLS}" PARENT_SCOPE)
endfunction()

# 配置编译器覆盖率标志
function(_configure_coverage_flags TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(WARNING "[coverage] Target ${TARGET_NAME} not found")
        return()
    endif()

    # GCC/Clang
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${TARGET_NAME} PRIVATE
            --coverage
            -g
            -O0
        )
        target_link_libraries(${TARGET_NAME} PRIVATE --coverage)
        message(STATUS "[coverage] Added GCC/Clang coverage flags to ${TARGET_NAME}")
    # MSVC - 使用 /DEBUG 生成调试信息
    elseif(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE
            /DEBUG
            /Od
            /Zi
        )
        target_link_options(${TARGET_NAME} PRIVATE
            /DEBUG
            /PROFILE
        )
        message(STATUS "[coverage] Added MSVC debug flags to ${TARGET_NAME}")
    endif()
endfunction()

# 使用 lcov 生成覆盖率报告
function(_generate_lcov_report)
    cmake_parse_arguments(
        GEN_LCOV
        ""
        "MODULE_NAME;OUTPUT_DIR;SOURCE_DIR"
        "EXCLUDE_PATTERNS"
        ${ARGN}
    )

    if(NOT GEN_LCOV_MODULE_NAME)
        message(WARNING "[coverage] No module name for lcov report")
        return()
    endif()

    if(NOT GEN_LCOV_OUTPUT_DIR)
        set(GEN_LCOV_OUTPUT_DIR "${CMAKE_BINARY_DIR}/coverage")
    endif()

    if(NOT GEN_LCOV_SOURCE_DIR)
        set(GEN_LCOV_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
    endif()

    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)

    if(NOT LCOV_EXECUTABLE OR NOT GENHTML_EXECUTABLE)
        message(WARNING "[coverage] lcov or genhtml not found")
        return()
    endif()

    file(MAKE_DIRECTORY "${GEN_LCOV_OUTPUT_DIR}")

    set(BASE_INFO_FILE "${GEN_LCOV_OUTPUT_DIR}/${GEN_LCOV_MODULE_NAME}_base.info")
    set(TEST_INFO_FILE "${GEN_LCOV_OUTPUT_DIR}/${GEN_LCOV_MODULE_NAME}_test.info")
    set(TOTAL_INFO_FILE "${GEN_LCOV_OUTPUT_DIR}/${GEN_LCOV_MODULE_NAME}_total.info")

    message(STATUS "[coverage] Generating lcov coverage report...")

    # 1. 捕获基础覆盖率数据（零计数）
    execute_process(
        COMMAND ${LCOV_EXECUTABLE}
            --capture
            --initial
            --directory "${CMAKE_BINARY_DIR}"
            --base-directory "${GEN_LCOV_SOURCE_DIR}"
            --output-file "${BASE_INFO_FILE}"
        RESULT_VARIABLE LCOV_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )

    # 2. 运行测试并捕获覆盖率数据
    if(EXISTS "${CMAKE_BINARY_DIR}/Testing")
        execute_process(
            COMMAND ${LCOV_EXECUTABLE}
                --capture
                --directory "${CMAKE_BINARY_DIR}"
                --base-directory "${GEN_LCOV_SOURCE_DIR}"
                --output-file "${TEST_INFO_FILE}"
            RESULT_VARIABLE LCOV_RESULT
            OUTPUT_QUIET
            ERROR_QUIET
        )
    endif()

    # 3. 合并基础和测试数据
    set(INFO_FILES "")
    if(EXISTS "${BASE_INFO_FILE}")
        list(APPEND INFO_FILES "${BASE_INFO_FILE}")
    endif()
    if(EXISTS "${TEST_INFO_FILE}")
        list(APPEND INFO_FILES "${TEST_INFO_FILE}")
    endif()

    if(INFO_FILES)
        execute_process(
            COMMAND ${LCOV_EXECUTABLE}
                --add-tracefile ${INFO_FILES}
                --output-file "${TOTAL_INFO_FILE}"
            RESULT_VARIABLE LCOV_RESULT
            OUTPUT_QUIET
            ERROR_QUIET
        )
    elseif(EXISTS "${BASE_INFO_FILE}")
        set(TOTAL_INFO_FILE "${BASE_INFO_FILE}")
    else()
        message(WARNING "[coverage] No coverage data collected")
        return()
    endif()

    # 4. 过滤不需要的文件
    set(FILTER_ARGS "")
    foreach(PATTERN ${GEN_LCOV_EXCLUDE_PATTERNS})
        list(APPEND FILTER_ARGS "--remove" "${TOTAL_INFO_FILE}" "${PATTERN}" "--output-file" "${TOTAL_INFO_FILE}")
    endforeach()

    # 默认过滤系统头文件和第三方库
    list(APPEND FILTER_ARGS "--remove" "${TOTAL_INFO_FILE}" "/usr/*" "--output-file" "${TOTAL_INFO_FILE}")
    list(APPEND FILTER_ARGS "--remove" "${TOTAL_INFO_FILE}" "*/tests/*" "--output-file" "${TOTAL_INFO_FILE}")
    list(APPEND FILTER_ARGS "--remove" "${TOTAL_INFO_FILE}" "*/examples/*" "--output-file" "${TOTAL_INFO_FILE}")

    execute_process(
        COMMAND ${LCOV_EXECUTABLE} ${FILTER_ARGS}
        RESULT_VARIABLE LCOV_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )

    # 5. 生成 HTML 报告
    set(HTML_OUTPUT_DIR "${GEN_LCOV_OUTPUT_DIR}/html")
    file(MAKE_DIRECTORY "${HTML_OUTPUT_DIR}")

    execute_process(
        COMMAND ${GENHTML_EXECUTABLE}
            "${TOTAL_INFO_FILE}"
            --output-directory "${HTML_OUTPUT_DIR}"
            --title "${GEN_LCOV_MODULE_NAME} Code Coverage"
            --show-details
            --legend
        RESULT_VARIABLE GENHTML_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )

    if(GENHTML_RESULT EQUAL 0)
        message(STATUS "[coverage] HTML coverage report generated: ${HTML_OUTPUT_DIR}/index.html")
    else()
        message(WARNING "[coverage] Failed to generate HTML report")
    endif()
endfunction()

# 使用 OpenCppCoverage 生成覆盖率报告 (Windows)
function(_generate_opencppcoverage_report)
    cmake_parse_arguments(
        GEN_OCP
        ""
        "MODULE_NAME;OUTPUT_DIR;TEST_EXECUTABLE"
        "EXCLUDE_PATTERNS"
        ${ARGN}
    )

    if(NOT WIN32)
        return()
    endif()

    if(NOT GEN_OCP_MODULE_NAME)
        message(WARNING "[coverage] No module name for OpenCppCoverage report")
        return()
    endif()

    if(NOT GEN_OCP_OUTPUT_DIR)
        set(GEN_OCP_OUTPUT_DIR "${CMAKE_BINARY_DIR}/coverage")
    endif()

    find_program(OPENCPPCOVERAGE_EXECUTABLE OpenCppCoverage)
    if(NOT OPENCPPCOVERAGE_EXECUTABLE)
        message(WARNING "[coverage] OpenCppCoverage not found")
        return()
    endif()

    if(NOT GEN_OCP_TEST_EXECUTABLE)
        # 尝试查找测试可执行文件
        file(GLOB_RECURSE TEST_EXES
            "${CMAKE_BINARY_DIR}/*_test*.exe"
            "${CMAKE_BINARY_DIR}/test*.exe"
        )
        if(TEST_EXES)
            list(GET TEST_EXES 0 GEN_OCP_TEST_EXECUTABLE)
        else()
            message(WARNING "[coverage] No test executable found")
            return()
        endif()
    endif()

    file(MAKE_DIRECTORY "${GEN_OCP_OUTPUT_DIR}")

    message(STATUS "[coverage] Running OpenCppCoverage...")

    set(OCP_ARGS
        "--sources" "${CMAKE_SOURCE_DIR}"
        "--export_type" "html:${GEN_OCP_OUTPUT_DIR}"
        "--working_dir" "${CMAKE_BINARY_DIR}"
        "--" "${GEN_OCP_TEST_EXECUTABLE}"
    )

    # 添加排除模式
    foreach(PATTERN ${GEN_OCP_EXCLUDE_PATTERNS})
        list(INSERT OCP_ARGS 0 "--excluded_sources" "${PATTERN}")
    endforeach()

    execute_process(
        COMMAND ${OPENCPPCOVERAGE_EXECUTABLE} ${OCP_ARGS}
        RESULT_VARIABLE OCP_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )

    if(OCP_RESULT EQUAL 0)
        message(STATUS "[coverage] OpenCppCoverage report generated: ${GEN_OCP_OUTPUT_DIR}/index.html")
    else()
        message(WARNING "[coverage] OpenCppCoverage failed with code: ${OCP_RESULT}")
    endif()
endfunction()

# 钩子分发函数
function(coverage_dispatch_hook)
    cmake_parse_arguments(
        DISPATCH
        ""
        "HOOK_NAME;MODULE_DIR;MODULE_NAME;MODULE_VERSION"
        ""
        ${ARGN}
    )

    if(DISPATCH_HOOK_NAME STREQUAL "pre_build")
        coverage_pre_build(
            MODULE_DIR "${DISPATCH_MODULE_DIR}"
            MODULE_NAME "${DISPATCH_MODULE_NAME}"
            MODULE_VERSION "${DISPATCH_MODULE_VERSION}"
        )
    elseif(DISPATCH_HOOK_NAME STREQUAL "post_build")
        coverage_post_build(
            MODULE_DIR "${DISPATCH_MODULE_DIR}"
            MODULE_NAME "${DISPATCH_MODULE_NAME}"
            MODULE_VERSION "${DISPATCH_MODULE_VERSION}"
        )
    endif()
endfunction()

# pre_build 钩子 - 配置覆盖率标志
function(coverage_pre_build)
    cmake_parse_arguments(
        PLUGIN_HOOK
        ""
        "MODULE_DIR;MODULE_NAME;MODULE_VERSION"
        ""
        ${ARGN}
    )

    message(STATUS "[coverage] Running pre_build hook for ${PLUGIN_HOOK_MODULE_NAME}")

    # 检查是否启用了 BUILD_TESTS
    if(NOT BUILD_TESTS)
        message(STATUS "[coverage] BUILD_TESTS is OFF, skipping coverage configuration")
        return()
    endif()

    message(STATUS "[coverage] Coverage plugin enabled, configuring build for coverage")
endfunction()

# post_build 钩子 - 生成覆盖率报告
function(coverage_post_build)
    cmake_parse_arguments(
        PLUGIN_HOOK
        ""
        "MODULE_DIR;MODULE_NAME;MODULE_VERSION"
        ""
        ${ARGN}
    )

    message(STATUS "[coverage] Running post_build hook for ${PLUGIN_HOOK_MODULE_NAME}")

    # 检查是否启用了 BUILD_TESTS
    if(NOT BUILD_TESTS)
        message(STATUS "[coverage] BUILD_TESTS is OFF, skipping coverage report")
        return()
    endif()

    # 查找可用的覆盖率工具
    _find_coverage_tools(COVERAGE_TOOLS)

    if(NOT COVERAGE_TOOLS)
        message(WARNING "[coverage] No coverage tools found, skipping report generation")
        return()
    endif()

    message(STATUS "[coverage] Available tools: ${COVERAGE_TOOLS}")

    # 尝试从 module.json 读取覆盖率配置
    set(MODULE_JSON_PATH "${PLUGIN_HOOK_MODULE_DIR}/module.json")
    set(EXCLUDE_PATTERNS "")

    if(EXISTS "${MODULE_JSON_PATH}")
        file(READ "${MODULE_JSON_PATH}" MODULE_JSON_CONTENT)

        string(JSON COVERAGE_CONFIG GET "${MODULE_JSON_CONTENT}" "coverage")
        if(COVERAGE_CONFIG)
            string(JSON EXCLUDE_LIST GET "${COVERAGE_CONFIG}" "exclude")
            if(EXCLUDE_LIST)
                string(JSON EXCLUDE_COUNT LENGTH "${EXCLUDE_LIST}")
                if(EXCLUDE_COUNT GREATER 0)
                    math(EXPR EXCLUDE_COUNT "${EXCLUDE_COUNT} - 1")
                    foreach(INDEX RANGE ${EXCLUDE_COUNT})
                        string(JSON PATTERN GET "${EXCLUDE_LIST}" ${INDEX})
                        list(APPEND EXCLUDE_PATTERNS "${PATTERN}")
                    endforeach()
                endif()
            endif()
        endif()
    endif()

    # 根据可用工具生成报告
    if("lcov" IN_LIST COVERAGE_TOOLS AND "genhtml" IN_LIST COVERAGE_TOOLS)
        _generate_lcov_report(
            MODULE_NAME "${PLUGIN_HOOK_MODULE_NAME}"
            SOURCE_DIR "${PLUGIN_HOOK_MODULE_DIR}"
            EXCLUDE_PATTERNS ${EXCLUDE_PATTERNS}
        )
    elseif("opencppcoverage" IN_LIST COVERAGE_TOOLS)
        _generate_opencppcoverage_report(
            MODULE_NAME "${PLUGIN_HOOK_MODULE_NAME}"
            EXCLUDE_PATTERNS ${EXCLUDE_PATTERNS}
        )
    else()
        message(WARNING "[coverage] No suitable coverage tool combination found")
    endif()

    message(STATUS "[coverage] post_build hook completed")
endfunction()
