add_executable(test_common
    test_common.c
)

target_include_directories(test_common PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/idcu-testframework/include
)

target_link_libraries(test_common
    idcu-common
    idcu-testframework
)

add_test(NAME test_common COMMAND test_common)
