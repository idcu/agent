add_executable(test_http_client
    test_http_client.c
)

target_include_directories(test_http_client PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/idcu-testframework/include
)

target_link_libraries(test_http_client
    idcu-http-client
    idcu-testframework
)

add_test(NAME test_http_client COMMAND test_http_client)
