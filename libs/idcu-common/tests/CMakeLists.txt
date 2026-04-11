add_executable(test_common
    test_common.c
)

target_link_libraries(test_common
    idcu-common
    idcu-testframework
)

add_test(NAME test_common COMMAND test_common)
