add_executable(test_os test_os.c)
target_link_libraries(test_os idcu::os)
add_test(NAME test_os COMMAND test_os)
