add_executable(test_yaml
    test_yaml.c
)

target_link_libraries(test_yaml
    PRIVATE
        idcu::yaml
        idcu::testframework
)

add_test(NAME test_yaml COMMAND test_yaml)
