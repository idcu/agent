add_executable(test_log
    test_log.c
)

target_link_libraries(test_log
    PRIVATE
        idcu::log
        idcu::testframework
)

add_test(NAME test_log COMMAND test_log)
