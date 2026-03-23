#include "future.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

void TestValue() {
    auto state = std::make_shared<SharedState<int>>();
    Future<int> future(state);

    std::thread worker([state]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        SetValue(state, 42);
    });

    int result = future.Get();
    assert(result == 42);

    worker.join();
    std::cout << "TestValue passed" << std::endl;
}

void TestException() {
    auto state = std::make_shared<SharedState<int>>();
    Future<int> future(state);

    std::thread worker([state]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        try {
            throw std::runtime_error("worker failed");
        } catch (...) {
            SetException(state, std::current_exception());
        }
    });

    bool caught = false;
    try {
        future.Get();
    } catch (const std::runtime_error& e) {
        caught = true;
        assert(std::string(e.what()) == "worker failed");
    }

    assert(caught);
    worker.join();
    std::cout << "TestException passed" << std::endl;
}

void TestVoidFuture() {
    auto state = std::make_shared<SharedState<void>>();
    Future<void> future(state);

    std::thread worker([state]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        SetValue(state);
    });

    future.Get();
    worker.join();
    std::cout << "TestVoidFuture passed" << std::endl;
}

void TestGetOnlyOnce() {
    auto state = std::make_shared<SharedState<int>>();
    Future<int> future(state);

    SetValue(state, 7);

    int result = future.Get();
    assert(result == 7);

    bool caught = false;
    try {
        future.Get();
    } catch (const std::runtime_error&) {
        caught = true;
    }

    assert(caught);
    std::cout << "TestGetOnlyOnce passed" << std::endl;
}

int main() {
    TestValue();
    TestException();
    TestVoidFuture();
    TestGetOnlyOnce();

    std::cout << "All future tests passed" << std::endl;
    return 0;
}