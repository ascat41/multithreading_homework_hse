#pragma once

#include <atomic>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

static int FutexWait(std::atomic<int>* addr, int expected) {
    return syscall(
        SYS_futex,
        reinterpret_cast<int*>(addr),
        FUTEX_WAIT,
        expected,
        nullptr,
        nullptr,
        0
    );
}

static int FutexWake(std::atomic<int>* addr, int count) {
    return syscall(
        SYS_futex,
        reinterpret_cast<int*>(addr),
        FUTEX_WAKE,
        count,
        nullptr,
        nullptr,
        0
    );
}

class FutexMutex {
public:
    FutexMutex() = default;
    // захват
    void lock() {
        int expected = Unlocked;
        if (state_.compare_exchange_strong(
                expected,
                LockedNoWaiters,
                std::memory_order_acquire,
                std::memory_order_relaxed)) {
            return;
        }

        while (state_.exchange(LockedWithWaiters, std::memory_order_acquire) != Unlocked) {
            FutexWait(&state_, LockedWithWaiters);
        }
    }
    // освободить
    void unlock() {
        if (state_.fetch_sub(1, std::memory_order_release) != LockedNoWaiters) {
            state_.store(Unlocked, std::memory_order_release);
            FutexWake(&state_, 1);
        }
    }
    // захват без ожидания
    bool try_lock() {
        int expected = Unlocked;
        // одна атомарная операция
        return state_.compare_exchange_strong(
            expected,
            LockedNoWaiters,
            std::memory_order_acquire,
            std::memory_order_relaxed
        );
    }

private:
    static constexpr int Unlocked = 0;
    static constexpr int LockedNoWaiters = 1;
    static constexpr int LockedWithWaiters = 2;
    // состояние 
    // 0 свободен
    // 1 захвачен без ожидающих
    // 2 захвачен и ожидающие 
    std::atomic<int> state_{Unlocked};
};