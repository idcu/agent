cmake_minimum_required(VERSION 3.15)

# Fuzzing tests - requires Clang with libFuzzer or AFL++

option(ENABLE_FUZZING "Enable fuzzing tests" OFF)

if(ENABLE_FUZZING)
    # Check for libFuzzer support
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-fsanitize=fuzzer" HAVE_LIBFUZZER)
    
    if(HAVE_LIBFUZZER)
        message(STATUS "Building fuzzing tests with libFuzzer")
        
        # Common fuzzing flags
        set(FUZZ_FLAGS "-fsanitize=fuzzer,address,undefined")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FUZZ_FLAGS} -g -O1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FUZZ_FLAGS} -g -O1")
        
        # JSON parser fuzz test
        add_executable(fuzz_json
            fuzz_json.c
        )
        target_link_libraries(fuzz_json
            idcu-json
            idcu-common
        )
        
        # YAML parser fuzz test
        add_executable(fuzz_yaml
            fuzz_yaml.c
        )
        target_link_libraries(fuzz_yaml
            idcu-yaml
            idcu-common
        )
        
        # HTTP parser fuzz test
        add_executable(fuzz_http
            fuzz_http.c
        )
        target_link_libraries(fuzz_http
            idcu-http-server
            idcu-common
        )
        
        # Message bus protocol fuzz test
        add_executable(fuzz_msgbus
            fuzz_msgbus.c
        )
        target_link_libraries(fuzz_msgbus
            idcu-msgbus
            idcu-common
        )
        
        # Config parser fuzz test
        add_executable(fuzz_config
            fuzz_config.c
        )
        target_link_libraries(fuzz_config
            idcu-config
            idcu-yaml
            idcu-json
            idcu-common
        )
        
    else()
        message(WARNING "libFuzzer not available, skipping fuzzing tests. "
                "Use Clang with -fsanitize=fuzzer support.")
    endif()
endif()
