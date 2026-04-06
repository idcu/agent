# Test file for CMake 4.2.3 string(JSON) syntax
message(STATUS "Testing CMake ${CMAKE_VERSION} string(JSON) syntax")

set(TEST_JSON "{\"plugins\": {\"version_generator\": true, \"packager\": false}}")
message(STATUS "Test JSON: ${TEST_JSON}")

# Test 1: Basic GET
string(JSON RESULT GET "${TEST_JSON}" "plugins")
message(STATUS "Test 1 - GET plugins: ${RESULT}")

# Test 2: Try ERROR_VARIABLE
# Let's see what the correct syntax is
# string(JSON ERROR_VARIABLE ERR GET "${TEST_JSON}" "nonexistent")
# message(STATUS "Test 2 - ERROR_VARIABLE: ${ERR}")

message(STATUS "JSON tests complete!")
