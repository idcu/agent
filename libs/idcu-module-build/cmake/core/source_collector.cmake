# 源文件收集模块
# 负责收集和管理源文件

# 收集源文件
function(collect_sources MODULE_DIR)
    cmake_parse_arguments(
        COLLECT_SRCS
        ""
        ""
        "SRCS_PATTERNS_VAR"
        ${ARGN}
    )

    if(COLLECT_SRCS_SRCS_PATTERNS_VAR)
        set(SRC_PATTERNS "${${COLLECT_SRCS_SRCS_PATTERNS_VAR}}")
    else()
        set(SRC_PATTERNS "${MODULE_DIR}/src/*.c")
    endif()

    file(GLOB_RECURSE COLLECTED_SOURCES
        ${SRC_PATTERNS}
    )

    set(COLLECTED_SOURCES "${COLLECTED_SOURCES}" PARENT_SCOPE)
endfunction()
