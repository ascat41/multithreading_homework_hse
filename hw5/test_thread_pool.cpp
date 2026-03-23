#include "thread_pool.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

void TestSimpleValues() {
    ThreadPool pool(3);

    auto future1 = pool.Submit([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 10;
    });

    auto future2 = pool.Submit([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return 32;
    });

    int result1 = future1.Get();
    int result2 = future2.Get();

    assert(result1 == 10);
    assert(result2 == 32);

    std::cout << "TestSimpleValues passed" << std::endl;
}

void TestExceptionFromTask() {
    ThreadPool pool(2);

    auto future = pool.Submit([]() -> int {
        throw std::runtime_error("task failed");
    });

    bool caught = false;
    try {
        future.Get();
    } catch (const std::runtime_error& e) {
        caught = true;
        assert(std::string(e.what()) == "task failed");
    }

    assert(caught);
    std::cout << "TestExceptionFromTask passed" << std::endl;
}

void TestVoidTask() {
    ThreadPool pool(2);

    int x = 0;
    auto future = pool.Submit([&x]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        x = 123;
    });

    future.Get();
    assert(x == 123);

    std::cout << "TestVoidTask passed" << std::endl;
}

void TestSubmitWithArgs() {
    ThreadPool pool(2);

    auto Add = [](int a, int b) {
        return a + b;
    };

    auto future = pool.Submit(Add, 20, 22);
    int result = future.Get();

    assert(result == 42);
    std::cout << "TestSubmitWithArgs passed" << std::endl;
}

void TestManyTasks() {
    ThreadPool pool(4);

    std::vector<Future<int>> futures;
    futures.reserve(20);

    for (int i = 0; i < 20; ++i) {
        futures.push_back(pool.Submit([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return i * i;
        }));
    }

    for (int i = 0; i < 20; ++i) {
        int result = futures[i].Get();
        assert(result == i * i);
    }

    std::cout << "TestManyTasks passed" << std::endl;
}

int main() {
    TestSimpleValues();
    TestExceptionFromTask();
    TestVoidTask();
    TestSubmitWithArgs();
    TestManyTasks();

    std::cout << "All thread pool tests passed" << std::endl;
    return 0;
}