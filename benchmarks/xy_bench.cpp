#include <benchmark/benchmark.h>

#include <cstdint>
#include <optional>

import aima.environment;

using namespace environment;

namespace {

// Deterministic population: every `stride`-th cell (row-major, 1-based) gets
// a dirt object. The agent is placed at the centre of the grid.
struct Fixture {
    XYEnvironment env;
    std::optional<TypeRef> agent;
};

Fixture makeFixture(int size, int stride) {
    Fixture f{makeXYEnvironment(size, size), std::nullopt};
    int n = 0;
    for (int y = 1; y <= size; ++y) {
        for (int x = 1; x <= size; ++x) {
            if (n++ % stride == 0) {
                addDirt(f.env, XYLocation(x, y));
            }
        }
    }
    f.agent = addAgent(f.env, XYLocation(size / 2, size / 2));
    return f;
}

void BM_GetObjectsNear(benchmark::State& state) {
    const int size = static_cast<int>(state.range(0));
    const int radius = static_cast<int>(state.range(1));
    const int stride = static_cast<int>(state.range(2));
    auto f = makeFixture(size, stride);

    for (auto _ : state) {
        auto near = getObjectsNear(f.agent, f.env, radius);
        benchmark::DoNotOptimize(near);
    }
    state.SetItemsProcessed(state.iterations());
}

// args: {grid size, radius, stride}; stride 50 = sparse, stride 2 = dense
BENCHMARK(BM_GetObjectsNear)
    ->Args({32, 3, 50})->Args({32, 3, 2})
    ->Args({128, 3, 50})->Args({128, 3, 2})
    ->Args({128, 8, 50})->Args({128, 8, 2})
    ->Args({256, 8, 50})->Args({256, 8, 2});

void BM_MoveObjectTo(benchmark::State& state) {
    const int size = static_cast<int>(state.range(0));
    auto env = makeXYEnvironment(size, size);
    const auto agent = addAgent(env, XYLocation(2, 2));
    const XYLocation a(2, 2), b(3, 3);
    bool toB = true;

    for (auto _ : state) {
        bool ok = moveObjectTo(agent, env, toB ? b : a);
        benchmark::DoNotOptimize(ok);
        toB = !toB;
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MoveObjectTo)->Arg(32)->Arg(256);

void BM_MoveObjectToCrowdedCell(benchmark::State& state) {
    const int crowd = static_cast<int>(state.range(0));
    auto env = makeXYEnvironment(32, 32);
    for (int i = 0; i < crowd; ++i) {
        addDirt(env, XYLocation(3, 3));
    }
    const auto agent = addAgent(env, XYLocation(2, 2));
    const XYLocation a(2, 2), b(3, 3);
    bool toB = true;

    for (auto _ : state) {
        bool ok = moveObjectTo(agent, env, toB ? b : a);
        benchmark::DoNotOptimize(ok);
        toB = !toB;
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MoveObjectToCrowdedCell)->Arg(10)->Arg(100);

void BM_MakeXYEnvironment(benchmark::State& state) {
    const int size = static_cast<int>(state.range(0));
    for (auto _ : state) {
        auto env = makeXYEnvironment(size, size);
        benchmark::DoNotOptimize(env);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MakeXYEnvironment)->Arg(32)->Arg(128);

void BM_IsBlocked(benchmark::State& state) {
    auto env = makeXYEnvironment(32, 32);
    addWall(env, XYLocation(5, 5));
    const XYLocation walled(5, 5), open(6, 6);
    bool toggle = false;

    for (auto _ : state) {
        bool blocked = isBlocked(env, toggle ? walled : open);
        benchmark::DoNotOptimize(blocked);
        toggle = !toggle;
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_IsBlocked);

}  // namespace
