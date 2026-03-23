#include <gtest/gtest.h>
#include <atomic>
#include <functional>
#include "apply_func.h"
#include <mutex>
#include <set>


// один поток
TEST(ApplyFunctionTest, SingleThread) {
    std::vector<int> data = {1, 2, 3, 4};

    std::function<void(int&)> transform = [](int& x) {
        x *= 2;
    };
    
    ApplyFunction(data, transform, 1);

    std::vector<int> expected = {2, 4, 6, 8};

    EXPECT_EQ(data, expected);
}

// пустой вектор
TEST(ApplyFunctionTest, EmptyVector) {
    std::vector<int> data;

    std::function<void(int&)> transform = [](int& x) {
        x *= 2;
    };

    ApplyFunction(data, transform, 2);

    EXPECT_TRUE(data.empty());
}

// потоков больше чем элементов
TEST(ApplyFunctionTest, MoreThreadsThanElements) {
    std::vector<int> data = {1, 2, 3};

    std::function<void(int&)> transform = [](int& x) {
        x += 1;
    };

    ApplyFunction(data, transform, 10);

    std::vector<int> expected = {2, 3, 4};
    EXPECT_EQ(data, expected);
}

// 0 потоков
TEST(ApplyFunctionTest, ZeroThreads) {
    std::vector<int> data = {1, 2, 3};

    std::function<void(int&)> transform = [](int& x) {
        x *= 3;
    };

    ApplyFunction(data, transform, 0);

    std::vector<int> expected = {3, 6, 9};
    EXPECT_EQ(data, expected);
}

// несколько потоков
TEST(ApplyFunctionTest, SomeThreads) {
    std::vector<int> data = {1, 2, 3, 4, 5, 6};

    std::function<void(int&)> transform = [](int& x) {
        x *= 2;
    };

    ApplyFunction(data, transform, 3);

    std::vector<int> expected = {2, 4, 6, 8, 10, 12};
    EXPECT_EQ(data, expected);
}

// каждый элемент обработан один раз
TEST(ApplyFunctionTest, EachElementOnce) {
    std::vector<int> data(1000, 1);

    std::atomic<int> calls{0};

    std::function<void(int&)> transform = [&](int& x) {
        x += 1;
        calls.fetch_add(1, std::memory_order_relaxed);
    };

    ApplyFunction(data, transform, 8);

    EXPECT_EQ(calls.load(std::memory_order_relaxed), 1000);

    for (int x : data) {
        EXPECT_EQ(x, 2);
    }
}

// используется примерно столько потоков сколько определено в вызове( не возможно проконтролировать на 100%)
TEST(ApplyFunctionTest, RightSomeThreads) {
    std::vector<int> data(10000, 1);

    std::mutex m;
    std::set<std::thread::id> ids;

    std::function<void(int&)> transform = [&](int& x) {
        x += 1;
        // получаем id потока и добавляем в список
        auto id = std::this_thread::get_id();
        std::lock_guard<std::mutex> lock(m);
        ids.insert(id);
    };

    ApplyFunction(data, transform, 4);

    // ids.size() показывает сколько разных потоков выполняли transform
    // может быть <= 4
    // но точно должно быть >= 1, ожидаем больше 3
    EXPECT_GE(ids.size(), 3u);

    // Чаще всего будет 4, но стабильно 4 не всегда гарантируется
}

// отрицательное число потоков
TEST(ApplyFunctionTest, MinusThreads) {
    std::vector<int> data = {1, 2, 3};

    std::function<void(int&)> transform = [](int& x) {
        x += 5;
    };

    ApplyFunction(data, transform, -5);

    std::vector<int> expected = {6, 7, 8};
    EXPECT_EQ(data, expected);
}