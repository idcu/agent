add_executable(test_cache
    test_cache.c
)

target_include_directories(test_cache PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/idcu-testframework/include
)

target_link_libraries(test_cache
    idcu-cache
    idcu-testframework
)

add_test(NAME test_cache COMMAND test_cache)
