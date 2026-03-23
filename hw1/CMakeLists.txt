cmake_minimum_required(VERSION 3.14)
project(hw1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.9.5.zip
)

FetchContent_MakeAvailable(googletest googlebenchmark)

enable_testing()

add_executable(apply_function_tests
    tests/test_apply_function.cpp
)

target_include_directories(apply_function_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(apply_function_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(apply_function_tests)

add_executable(apply_function_benchmarks
    benchmarks/benchmark_apply_function.cpp
)

target_include_directories(apply_function_benchmarks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(apply_function_benchmarks benchmark::benchmark_main)