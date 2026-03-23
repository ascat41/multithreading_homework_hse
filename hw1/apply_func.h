#pragma once

#include <vector>
#include <functional>
#include <thread>
#include <cstddef> 


template <typename T>
void ApplyFunction(std::vector<T>& data,
                     const std::function<void(T&)>& transform,
                     const int threadCount = 1){
    if (data.empty()) {
        return;
    }
    // It seems we can change the type to size_t
    int effectiveThreads = threadCount;
    // Zero threads case
    if (effectiveThreads < 1) {
        effectiveThreads = 1;
    }
    // More threads than elements
    if (effectiveThreads > static_cast<int>(data.size())) {
        effectiveThreads = static_cast<int>(data.size());
    }
    // Lambda to process a range of the vector
    auto processRange = [&](std::size_t start, std::size_t end) {
        for (std::size_t i = start; i < end; ++i) {
            transform(data[i]);
        }
    };
    // If single thread
    if (effectiveThreads == 1) {
        processRange(0, data.size());
        return;
    }
    // Create vector of threads and determine how many elements per thread
    std::vector<std::thread> workerThreads;
    // Reserve memory
    workerThreads.reserve(effectiveThreads);
    // Number of elements per thread
    std::size_t chunkSize = data.size() / effectiveThreads;
    std::size_t remainder = data.size() % effectiveThreads;
    std::size_t startIndex = 0;
    for (int t = 0; t < effectiveThreads; ++t) {
        std::size_t currentChunkSize = chunkSize;
        // Distribute remainder to first threads
        if (t < static_cast<int>(remainder)) {
            currentChunkSize += 1;
        }
        std::size_t endIndex = startIndex + currentChunkSize;
        // Create new thread
        workerThreads.emplace_back(processRange, startIndex, endIndex);
        // Shift start for next thread
        startIndex = endIndex;
    }
    // Wait for all threads to finish
    for (auto& thread : workerThreads) {
        thread.join();
    }
}