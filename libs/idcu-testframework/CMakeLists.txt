add_library(idcu_testframework STATIC
    src/testframework.c
)

target_include_directories(idcu_testframework PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(idcu::testframework ALIAS idcu_testframework)

if(BUILD_EXAMPLES)
    add_executable(example_simple_test examples/simple_test.c)
    target_link_libraries(example_simple_test PRIVATE idcu::testframework)
endif()
