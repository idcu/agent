cmake_minimum_required(VERSION 3.15)
project(idcu-os C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(IDCU_OS_SOURCES)

if(WIN32)
    list(APPEND IDCU_OS_SOURCES
        src/windows/os.c
        src/windows/lock.c
        src/windows/time.c
        src/windows/network.c
        src/windows/file.c
        src/windows/path.c
        src/windows/env.c
        src/windows/dl.c
        src/windows/sysinfo.c
    )
elseif(APPLE)
    list(APPEND IDCU_OS_SOURCES
        src/macos/os.c
        src/macos/lock.c
        src/macos/time.c
        src/macos/network.c
        src/macos/file.c
        src/macos/path.c
        src/macos/env.c
        src/macos/dl.c
        src/macos/sysinfo.c
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    list(APPEND IDCU_OS_SOURCES
        src/freebsd/os.c
        src/freebsd/lock.c
        src/freebsd/time.c
        src/freebsd/network.c
        src/freebsd/file.c
        src/freebsd/path.c
        src/freebsd/env.c
        src/freebsd/dl.c
        src/freebsd/sysinfo.c
    )
elseif(ANDROID)
    list(APPEND IDCU_OS_SOURCES
        src/android/os.c
        src/android/lock.c
        src/android/time.c
        src/android/network.c
        src/android/file.c
        src/android/path.c
        src/android/env.c
        src/android/dl.c
        src/android/sysinfo.c
    )
else()
    list(APPEND IDCU_OS_SOURCES
        src/linux/os.c
        src/linux/lock.c
        src/linux/time.c
        src/linux/network.c
        src/linux/file.c
        src/linux/path.c
        src/linux/env.c
        src/linux/dl.c
        src/linux/sysinfo.c
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
