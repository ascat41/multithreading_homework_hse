cmake_minimum_required(VERSION 3.22)
project(hw4_ipc_queue CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ipc_queue_lib ipc_queue.cpp)
target_include_directories(ipc_queue_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(producer_demo producer_demo.cpp)
target_link_libraries(producer_demo PRIVATE ipc_queue_lib)

add_executable(consumer_demo consumer_demo.cpp)
target_link_libraries(consumer_demo PRIVATE ipc_queue_lib)
