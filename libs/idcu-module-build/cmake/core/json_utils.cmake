# JSON 工具模块
# 提供 JSON 安全访问等工具函数

include(CMakeParseArguments)

# 安全获取 JSON 成员 - 如果不存在返回空字符串
function(safe_json_get RESULT_VAR JSON_CONTENT)
    cmake_parse_arguments(
        SAFE_JSON
        ""
        ""
        "KEYS"
        ${ARGN}
    )

    set(CURRENT_JSON "${JSON_CONTENT}")
    set(FOUND TRUE)

    foreach(KEY ${SAFE_JSON_KEYS})
        # 先检查键是否存在
        string(JSON MEMBER_COUNT LENGTH "${CURRENT_JSON}")
        set(KEY_EXISTS FALSE)
        math(EXPR LAST_INDEX "${MEMBER_COUNT} - 1")
        foreach(I RANGE ${LAST_INDEX})
            string(JSON MEMBER_NAME MEMBER "${CURRENT_JSON}" ${I})
            if(MEMBER_NAME STREQUAL KEY)
                set(KEY_EXISTS TRUE)
                break()
            endif()
        endforeach()
        
        if(NOT KEY_EXISTS)
            set(FOUND FALSE)
            break()
        endif()
        
        string(JSON TEMP GET "${CURRENT_JSON}" "${KEY}")
        set(CURRENT_JSON "${TEMP}")
    endforeach()

    if(FOUND)
        set(${RESULT_VAR} "${CURRENT_JSON}" PARENT_SCOPE)
    else()
        set(${RESULT_VAR} "" PARENT_SCOPE)
    endif()
endfunction()
