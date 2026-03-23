#pragma once

#include <optional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <utility>



template <class T>
class BufferedChannel {
public:
    explicit BufferedChannel(int size) : size_(size) {
    }

    void Send(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);

        not_full_.wait(lock, [this]() {
            return closed_ || static_cast<int>(queue_.size()) < size_;
        });

        if (closed_) {
            throw std::runtime_error("Channel closed");
        }

        queue_.push(value);
        not_empty_.notify_one();
    }

    std::optional<T> Recv() {
        std::unique_lock<std::mutex> lock(mutex_);

        not_empty_.wait(lock, [this]() {
            return closed_ || !queue_.empty();
        });

        // канал закрыт
        if (queue_.empty()) {
            return std::nullopt;
        }

        T value = std::move(queue_.front());
        queue_.pop();
        not_full_.notify_one();

        return value;
    }

    void Close() {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;
        not_empty_.notify_all();
        not_full_.notify_all();
    }
private:
    int size_;
    //очередь элементов
    std::queue<T> queue_;
    bool closed_ = false;
    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
};
