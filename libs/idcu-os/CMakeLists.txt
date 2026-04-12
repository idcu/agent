cmake_minimum_required(VERSION 3.15)
project(idcu-os C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(IDCU_OS_SOURCES)

if(WIN32)
    list(APPEND IDCU_OS_SOURCES
        src/idcu/os/windows/os.c
        src/idcu/os/windows/lock.c
        src/idcu/os/windows/time.c
        src/idcu/os/windows/network.c
        src/idcu/os/windows/file.c
        src/idcu/os/windows/path.c
        src/idcu/os/windows/env.c
        src/idcu/os/windows/dl.c
        src/idcu/os/windows/sysinfo.c
    )
elseif(APPLE)
    list(APPEND IDCU_OS_SOURCES
        src/idcu/os/macos/os.c
        src/idcu/os/macos/lock.c
        src/idcu/os/macos/time.c
        src/idcu/os/macos/network.c
        src/idcu/os/macos/file.c
        src/idcu/os/macos/path.c
        src/idcu/os/macos/env.c
        src/idcu/os/macos/dl.c
        src/idcu/os/macos/sysinfo.c
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    list(APPEND IDCU_OS_SOURCES
        src/idcu/os/freebsd/os.c
        src/idcu/os/freebsd/lock.c
        src/idcu/os/freebsd/time.c
        src/idcu/os/freebsd/network.c
        src/idcu/os/freebsd/file.c
        src/idcu/os/freebsd/path.c
        src/idcu/os/freebsd/env.c
        src/idcu/os/freebsd/dl.c
        src/idcu/os/freebsd/sysinfo.c
    )
elseif(ANDROID)
    list(APPEND IDCU_OS_SOURCES
        src/idcu/os/android/os.c
        src/idcu/os/android/lock.c
        src/idcu/os/android/time.c
        src/idcu/os/android/network.c
        src/idcu/os/android/file.c
        src/idcu/os/android/path.c
        src/idcu/os/android/env.c
        src/idcu/os/android/dl.c
        src/idcu/os/android/sysinfo.c
    )
else()
    list(APPEND IDCU_OS_SOURCES
        src/idcu/os/linux/os.c
        src/idcu/os/linux/lock.c
        src/idcu/os/linux/time.c
        src/idcu/os/linux/network.c
        src/idcu/os/linux/file.c
        src/idcu/os/linux/path.c
        src/idcu/os/linux/env.c
        src/idcu/os/linux/dl.c
        src/idcu/os/linux/sysinfo.c
    )
endif()

add_library(idcu-os STATIC ${IDCU_OS_SOURCES})

target_include_directories(idcu-os PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(WIN32)
    target_link_libraries(idcu-os PRIVATE ws2_32 advapi32 shlwapi)
elseif(APPLE)
    target_link_libraries(idcu-os PRIVATE pthread dl)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    target_link_libraries(idcu-os PRIVATE pthread rt dl kvm)
elseif(ANDROID)
    target_link_libraries(idcu-os PRIVATE log android)
else()
    target_link_libraries(idcu-os PRIVATE pthread dl rt)
endif()

add_library(idcu::os ALIAS idcu-os)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
