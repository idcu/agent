# Monitor 服务模块测试 CMakeLists.txt

file(GLOB_RECURSE TEST_SRCS
    test_*.c
)

foreach(test_src ${TEST_SRCS})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_include_directories(${test_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../core/common/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../core/utils/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../core/scheduler/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../services/monitor/include
    )
    target_link_libraries(${test_name} PRIVATE
        idcu_services_monitor
        idcu_core_scheduler
        idcu_core_test_framework
    )
    if(WIN32)
        target_link_libraries(${test_name} PRIVATE -lws2_32)
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
