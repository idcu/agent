add_executable(test_healthcheck
    test_healthcheck.c
)

target_include_directories(test_healthcheck PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/idcu-testframework/include
)

target_link_libraries(test_healthcheck
    idcu-healthcheck
    idcu-testframework
)

add_test(NAME test_healthcheck COMMAND test_healthcheck)
