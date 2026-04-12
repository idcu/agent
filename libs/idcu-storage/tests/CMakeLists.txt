add_executable(test_storage
    test_storage.c
)

target_include_directories(test_storage PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/idcu-testframework/include
)

target_link_libraries(test_storage
    idcu-storage
    idcu-testframework
)

add_test(NAME test_storage COMMAND test_storage)
