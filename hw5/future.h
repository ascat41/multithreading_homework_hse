#pragma once

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <class T>
struct SharedState {
    std::mutex state_mutex;
    std::condition_variable state_cv;
    std::optional<T> result;
    std::exception_ptr stored_exception;
    bool is_ready = false;
    bool is_retrieved = false;
};

template <>
struct SharedState<void> {
    std::mutex state_mutex;
    std::condition_variable state_cv;
    std::exception_ptr stored_exception;
    bool is_ready = false;
    bool is_retrieved = false;
};

template <class T>
class Future {
public:
    Future() = default;

    explicit Future(std::shared_ptr<SharedState<T>> state)
        : shared_state_(std::move(state)) {
    }

    void Wait() const {
        if (!shared_state_) {
            throw std::runtime_error("Future has no state");
        }

        std::unique_lock<std::mutex> lock(shared_state_->state_mutex);
        shared_state_->state_cv.wait(lock, [this]() {
            return shared_state_->is_ready;
        });
    }

    T Get() {
        Wait();

        std::lock_guard<std::mutex> lock(shared_state_->state_mutex);

        if (shared_state_->is_retrieved) {
            throw std::runtime_error("Future result already retrieved");
        }

        shared_state_->is_retrieved = true;

        if (shared_state_->stored_exception) {
            std::rethrow_exception(shared_state_->stored_exception);
        }

        if (!shared_state_->result.has_value()) {
            throw std::runtime_error("SharedState has no value");
        }

        T result = std::move(*(shared_state_->result));
        shared_state_->result.reset();
        return result;
    }

private:
    std::shared_ptr<SharedState<T>> shared_state_;
};

template <>
class Future<void> {
public:
    Future() = default;

    explicit Future(std::shared_ptr<SharedState<void>> state)
        : shared_state_(std::move(state)) {
    }

    void Wait() const {
        if (!shared_state_) {
            throw std::runtime_error("Future has no state");
        }

        std::unique_lock<std::mutex> lock(shared_state_->state_mutex);
        shared_state_->state_cv.wait(lock, [this]() {
            return shared_state_->is_ready;
        });
    }

    void Get() {
        Wait();

        std::lock_guard<std::mutex> lock(shared_state_->state_mutex);

        if (shared_state_->is_retrieved) {
            throw std::runtime_error("Future result already retrieved");
        }

        shared_state_->is_retrieved = true;

        if (shared_state_->stored_exception) {
            std::rethrow_exception(shared_state_->stored_exception);
        }
    }

private:
    std::shared_ptr<SharedState<void>> shared_state_;
};

template <class T>
void SetValue(const std::shared_ptr<SharedState<T>>& state, T value) {
    {
        std::lock_guard<std::mutex> lock(state->state_mutex);

        if (state->is_ready) {
            throw std::runtime_error("SharedState already satisfied");
        }

        state->result = std::move(value);
        state->is_ready = true;
    }

    state->state_cv.notify_all();
}

inline void SetValue(const std::shared_ptr<SharedState<void>>& state) {
    {
        std::lock_guard<std::mutex> lock(state->state_mutex);

        if (state->is_ready) {
            throw std::runtime_error("SharedState already satisfied");
        }

        state->is_ready = true;
    }

    state->state_cv.notify_all();
}

template <class T>
void SetException(const std::shared_ptr<SharedState<T>>& state, std::exception_ptr exception) {
    {
        std::lock_guard<std::mutex> lock(state->state_mutex);

        if (state->is_ready) {
            throw std::runtime_error("SharedState already satisfied");
        }

        state->stored_exception = exception;
        state->is_ready = true;
    }

    state->state_cv.notify_all();
}

inline void SetException(const std::shared_ptr<SharedState<void>>& state, std::exception_ptr exception) {
    {
        std::lock_guard<std::mutex> lock(state->state_mutex);

        if (state->is_ready) {
            throw std::runtime_error("SharedState already satisfied");
        }

        state->stored_exception = exception;
        state->is_ready = true;
    }

    state->state_cv.notify_all();
}