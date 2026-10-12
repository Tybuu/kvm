cmake_minimum_required(VERSION 3.13)
project(host_tests C)

set(CMAKE_C_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.0
)
FetchContent_MakeAvailable(unity)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
target_compile_definitions(unity PUBLIC UNITY_INCLUDE_PRINT_FORMATTED)

# The unit under test — only the decoupled logic, no SDK includes
add_library(emulate_logic STATIC
    ${CMAKE_SOURCE_DIR}/../src/emulate.c
)
target_include_directories(emulate_logic PUBLIC ${CMAKE_SOURCE_DIR}/../include)

# One test executable per module (or combine, your call)
add_executable(test_emulate test_emulate.c)
target_link_libraries(test_emulate emulate_logic unity)

# Register with ctest so `ctest` runs everything
enable_testing()
add_test(NAME test_emulate COMMAND test_emulate)
