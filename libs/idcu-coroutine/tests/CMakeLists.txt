add_executable(coroutine_test
    test_coroutine.c
)

target_link_libraries(coroutine_test
    PRIVATE
        idcu::coroutine
        idcu::testframework
)

add_test(NAME coroutine_test COMMAND coroutine_test)
