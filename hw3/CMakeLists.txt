cmake_minimum_required(VERSION 3.22)
project(hw3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(test_futex_mutex test_futex_mutex.cpp)
target_include_directories(test_futex_mutex PRIVATE .)
target_link_libraries(test_futex_mutex PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_futex_mutex)