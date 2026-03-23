#include <gtest/gtest.h>
#include "futex_mutex.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

TEST(FutexMutexTest, TryLockUnlockTryLock) {
    FutexMutex lock_guard;

    EXPECT_TRUE(lock_guard.try_lock());
    EXPECT_FALSE(lock_guard.try_lock());

    lock_guard.unlock();

    EXPECT_TRUE(lock_guard.try_lock());
    lock_guard.unlock();
}

// проверка блокирования другого потока до разблокировки
TEST(FutexMutexTest, LockBlocksOtherThreadUntilUnlock) {
    FutexMutex lock_guard;
    std::atomic<bool> otherThreadGotLock{false};

    lock_guard.lock();

    std::thread t([&]() {
        lock_guard.lock();
        otherThreadGotLock.store(true, std::memory_order_relaxed);
        lock_guard.unlock();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(otherThreadGotLock.load(std::memory_order_relaxed));

    lock_guard.unlock();

    t.join();

    EXPECT_TRUE(otherThreadGotLock.load(std::memory_order_relaxed));
}

// гонка потоков с общим счетчиком
TEST(FutexMutexTest, ProtectsSharedCounter) {
    FutexMutex lock_guard;
    int shared_count = 0;

    constexpr int kThreadCount = 4;
    constexpr int kIterationsPerThread = 10000;

    std::vector<std::thread> worker_threads;
    worker_threads.reserve(kThreadCount);

    for (int worker_index = 0; worker_index < kThreadCount; ++worker_index) {
        worker_threads.emplace_back([&]() {
            for (int iter = 0; iter < kIterationsPerThread; ++iter) {
                lock_guard.lock();
                ++shared_count;
                lock_guard.unlock();
            }
        });
    }

    for (auto& worker : worker_threads) {
        worker.join();
    }

    EXPECT_EQ(shared_count, kThreadCount * kIterationsPerThread);
}