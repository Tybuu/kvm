cmake_minimum_required(VERSION 3.20)
project(PicoKvm CXX)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED IMPORTED_TARGET libevdev)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_Declare(
    hidapi
    GIT_REPOSITORY https://github.com/libusb/hidapi.git
    GIT_TAG master
)
FetchContent_MakeAvailable(googletest hidapi)

add_executable(PicoKvm src/main.cpp)
target_include_directories(PicoKvm PRIVATE include)
target_link_libraries(PicoKvm PRIVATE hidapi::hidapi PkgConfig::LIBEVDEV)

add_executable(PicoKvmTests tests/test_main.cpp)
target_link_libraries(PicoKvmTests PRIVATE gtest_main)
include(GoogleTest)
gtest_discover_tests(PicoKvmTests)
