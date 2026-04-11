add_executable(test_network
    test_network.c
)

target_link_libraries(test_network
    PRIVATE
        idcu::network
        idcu::testframework
)

add_test(NAME test_network COMMAND test_network)
