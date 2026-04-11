add_executable(test_core_infrastructure
    test_core_infrastructure.c
)

target_link_libraries(test_core_infrastructure
    PRIVATE
        idcu::module-system
        idcu::coroutine
        idcu::msgbus
        idcu::testframework
)

add_test(NAME test_core_infrastructure COMMAND test_core_infrastructure)

add_executable(test_microkernel
    test_microkernel.c
)

target_link_libraries(test_microkernel
    PRIVATE
        idcu::microkernel
        idcu::testframework
)

add_test(NAME test_microkernel COMMAND test_microkernel)
