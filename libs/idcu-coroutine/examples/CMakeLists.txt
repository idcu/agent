add_executable(simple_coroutine
    simple_coroutine.c
)

target_link_libraries(simple_coroutine
    PRIVATE
        idcu::coroutine
)
