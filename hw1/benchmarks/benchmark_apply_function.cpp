#include <benchmark/benchmark.h>
#include "apply_func.h"


static void BM_Single_Light_SmallVector(benchmark::State& state) {
    // Get vector length
    const std::size_t n = static_cast<std::size_t>(state.range(0));

    std::function<void(int&)> operation = [](int& x) {
        x += 1;
    };

    for (auto _ : state) {
        // Create vector of length n and apply ApplyFunction with 1 thread
        std::vector<int> inputData(n, 1);
        ApplyFunction(inputData, operation, 1);
        benchmark::DoNotOptimize(inputData);
    }
}
// Use UseRealTime to evaluate actual time rather than
// CPU time consumed by all threads
BENCHMARK(BM_Single_Light_SmallVector)->Arg(100)->UseRealTime();

static void BM_Multi_Light_SmallVector(benchmark::State& state) {
    const std::size_t n = static_cast<std::size_t>(state.range(0));

    std::function<void(int&)> operation = [](int& x) {
        x += 1;
    };

    for (auto _ : state) {
        std::vector<int> inputData(n, 1);
        ApplyFunction(inputData, operation, 4);
        benchmark::DoNotOptimize(inputData);
    }
}
BENCHMARK(BM_Multi_Light_SmallVector)->Arg(100)->UseRealTime();

static void BM_Single_Heavy_BigVector(benchmark::State& state) {
    const std::size_t n = static_cast<std::size_t>(state.range(0));

    std::function<void(int&)> operation = [](int& x) {
        for (int i = 0; i < 1000; ++i) {
            x = (x * 1664525 + 1013904223) % 1000000007;
        }
    };

    for (auto _ : state) {
        std::vector<int> inputData(n, 1);
        ApplyFunction(inputData, operation, 1);
        benchmark::DoNotOptimize(inputData);
    }
}
BENCHMARK(BM_Single_Heavy_BigVector)->Arg(100000)->UseRealTime();

static void BM_Multi_Heavy_BigVector(benchmark::State& state) {
    const std::size_t n = static_cast<std::size_t>(state.range(0));

    std::function<void(int&)> operation = [](int& x) {
        for (int i = 0; i < 1000; ++i) {
            x = (x * 1664525 + 1013904223) % 1000000007;
        }
    };

    for (auto _ : state) {
        std::vector<int> inputData(n, 1);
        ApplyFunction(inputData, operation, 4);
        benchmark::DoNotOptimize(inputData);
    }
}
BENCHMARK(BM_Multi_Heavy_BigVector)->Arg(100000)->UseRealTime();

BENCHMARK_MAIN();