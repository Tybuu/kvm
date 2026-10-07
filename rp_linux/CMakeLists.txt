cmake_minimum_required(VERSION 3.20)
project(PicoKvm CXX)

set(CMAKE_CXX_STANDARD 17)

option(BUILD_TESTING "Build test suite" ON)

if(CMAKE_CROSSCOMPILING OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--dynamic-linker=/lib/ld-linux-aarch64.so.1")
    set(BUILD_TESTING OFF CACHE BOOL "Build test suite" FORCE)
endif()

if(BUILD_TESTING)
    enable_testing()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED IMPORTED_TARGET libevdev)
pkg_check_modules(LIBUDEV REQUIRED IMPORTED_TARGET libudev)

include(FetchContent)
if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
endif()
FetchContent_Declare(
    hidapi
    GIT_REPOSITORY https://github.com/libusb/hidapi.git
    GIT_TAG master
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

if(BUILD_TESTING)
    FetchContent_MakeAvailable(googletest hidapi)
    add_executable(PicoKvm src/host_main.cpp ./src/hid_commands.cpp ./src/poll.cpp)
else()
    add_executable(PicoKvm src/main.cpp src/uart.cpp ./src/hid_commands.cpp ./src/poll.cpp)
    FetchContent_MakeAvailable(hidapi)
endif()

target_include_directories(PicoKvm PRIVATE include)
target_link_libraries(PicoKvm PRIVATE hidapi::hidapi PkgConfig::LIBEVDEV PkgConfig::LIBUDEV)

if(BUILD_TESTING)
    add_executable(PicoKvmTests tests/test_main.cpp)
    target_link_libraries(PicoKvmTests PRIVATE gtest_main)
    include(GoogleTest)
    gtest_discover_tests(PicoKvmTests)
endif()
