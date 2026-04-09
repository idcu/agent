# app/CMakeLists.txt

add_executable(idcu_agent
    main.c
    src/version.c
)

target_include_directories(idcu_agent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(TARGETS idcu_agent
    RUNTIME DESTINATION bin
)
