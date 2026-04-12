add_executable(test_http_server
    test_http_server.c
)

target_include_directories(test_http_server PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/idcu-testframework/include
)

target_link_libraries(test_http_server
    idcu-http-server
    idcu-testframework
)

add_test(NAME test_http_server COMMAND test_http_server)
