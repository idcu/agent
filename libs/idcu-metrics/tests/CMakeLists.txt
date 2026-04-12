add_executable(test_metrics
    test_metrics.c
)

target_include_directories(test_metrics PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/idcu-testframework/include
)

target_link_libraries(test_metrics
    idcu-metrics
    idcu-testframework
)

add_test(NAME test_metrics COMMAND test_metrics)
