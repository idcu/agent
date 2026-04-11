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

add_executable(integration_test_microkernel
    test_microkernel.c
)

target_link_libraries(integration_test_microkernel
    PRIVATE
        idcu::microkernel
        idcu::testframework
)

add_test(NAME integration_test_microkernel COMMAND integration_test_microkernel)
