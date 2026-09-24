# Beginner Refactoring Guide: Functional/ECS-Style Vacuum Agent

One implementation step per commit. Each commit should compile, pass tests, and be independently reviewable.

---

## Commit-by-Commit Checklist

### Commit 1 — Add value types (no behavior yet)
- [ ] Add `Location`, `loc_A`, `loc_B`.
- [ ] Add `Cleanliness` enum.
- [ ] Add `Action` enum (replaces string actions).
- [ ] Add `VacuumPercept` struct.
- [ ] Build only — no tests yet, nothing uses these types.

### Commit 2 — Add data-only structs
- [ ] Add `RandomVacuumAgent { Location location; int performance; }`.
- [ ] Add `TrivialVacuumEnvironment { std::array<Cleanliness, 2> status; }`.
- [ ] No constructors, no methods, no references.
- [ ] Build only.

### Commit 3 — Add location index + accessor tests
- [ ] Implement `location_index(Location) -> std::size_t`.
- [ ] Implement `cleanliness_at(...)`.
- [ ] Implement `set_cleanliness(...)`.
- [ ] Add unit tests for all three.
- [ ] Run tests: `ctest --test-dir build --output-on-failure`.

### Commit 4 — Add environment factory
- [ ] Implement `random_cleanliness(std::mt19937&)`.
- [ ] Implement `make_trivial_vacuum_environment(std::mt19937&)`.
- [ ] Add a test with a seeded generator.
- [ ] Confirm the environment stores no generator reference.

### Commit 5 — Add perception
- [ ] Implement `perceive(environment, agent) -> VacuumPercept`.
- [ ] Add a unit test.

### Commit 6 — Add action application
- [ ] Implement `apply_action(environment, agent, action)` with a `switch`.
- [ ] Add tests: `Suck` on dirty, `Suck` on clean, `Right`, `Left`.

### Commit 7 — Add a decision function type + random functor
- [ ] Define `DecisionFn` (start with `std::function`).
- [ ] Implement `RandomActionSystem` as a stateless functor.
- [ ] Add a test that asserts only valid `Action` values are produced.

### Commit 8 — Add a single-system tick function
- [ ] Implement `make_agent_tick(DecisionFn) -> WorldTickFn`.
- [ ] Add a test using a scripted lambda decision (deterministic, not random).

### Commit 9 — Add the system pipeline + run loop
- [ ] Define `SystemPipeline = std::vector<WorldTickFn>`.
- [ ] Implement `run(environment, agent, generator, pipeline, steps)`.
- [ ] Add a test proving systems execute in vector order.

### Commit 10 — Rewrite the integration test
- [ ] Replace the old class-based `RandomVacuumAgentTest` with the new stack-only version using `run()` + `pipeline`.
- [ ] Confirm no `std::shared_ptr`, no inheritance, no heap-allocated agent/environment.

### Commit 11 — Delete old OOP code
- [ ] Delete `Thing`, `Agent`, `Environment` base classes.
- [ ] Delete `RandomAgentProgram` (old form).
- [ ] Remove now-unused includes (`<memory>` if no longer needed).
- [ ] Full clean rebuild + Valgrind check.

```bash
rm -rf build && cmake -S . -B build && cmake --build build
ctest --test-dir build --output-on-failure
valgrind --tool=memcheck --track-origins=yes --leak-check=full \
  ./build/tests/unit_tests/aima_unit_tests
```

Expected: `ERROR SUMMARY: 0 errors from 0 contexts`.

---

## Comparison: Ways to Represent "a function that can vary at runtime/compile-time"

You need a `DecisionFn`-like thing (`Action(agent, percept, generator)`). Here are four ways to implement it, in increasing order of restriction and decreasing order of runtime flexibility.

### 1. `std::function` (most flexible, some heap allocation risk)

```cpp
using DecisionFn = std::function<Action(
    const RandomVacuumAgent&,
    const VacuumPercept&,
    std::mt19937&)>;

std::vector<DecisionFn> systems{ RandomActionSystem{}, reflex_decision };
```

- ✅ Can hold lambdas with captures, functors, or free functions — all with the same type.
- ✅ Can be stored in a `std::vector`, changed at runtime, loaded from config, etc.
- ❌ May heap-allocate if the callable exceeds small-buffer-optimization size (implementation-defined).
- ❌ Virtual-call-like indirection (type erasure) — slightly slower than a direct call.
- **Best for:** runtime-composable systems, plugin-style behavior, this project's learning goal.

### 2. Function pointers (zero allocation, but no captures)

```cpp
using DecisionFn = Action(*)(
    const RandomVacuumAgent&,
    const VacuumPercept&,
    std::mt19937&);

Action random_decision(
    const RandomVacuumAgent&, const VacuumPercept&, std::mt19937& gen) {
    // ...
}

std::array<DecisionFn, 1> systems{ random_decision };
```

- ✅ Zero heap allocation, trivially copyable, very fast.
- ❌ Cannot capture state (no lambda captures, no functor member data).
- ❌ Every "system" must be a free function or a capture-less lambda (implicitly convertible to a function pointer).
- **Best for:** strict zero-allocation requirements when systems are stateless.

### 3. Templates (compile-time polymorphism, zero overhead)

```cpp
template <typename Decision>
void run_with_decision(
    TrivialVacuumEnvironment& environment,
    RandomVacuumAgent& agent,
    std::mt19937& generator,
    Decision decide,
    int steps) {
    for (int i = 0; i < steps; ++i) {
        const auto percept = perceive(environment, agent);
        const auto action = decide(agent, percept, generator);
        apply_action(environment, agent, action);
    }
}

run_with_decision(environment, agent, generator, RandomActionSystem{}, 1000);
```

- ✅ Zero runtime overhead — fully inlined, no indirection.
- ✅ Works with any callable (lambda, functor, function pointer) via deduction — no wrapper type needed.
- ❌ Not runtime-swappable — the decision is fixed at compile time per instantiation.
- ❌ Increases compile time and binary size (one instantiation per distinct callable type).
- **Best for:** performance-critical single-strategy simulations, or when the system list is known at compile time.

### 4. `std::variant` of known functors (bounded runtime polymorphism, zero allocation)

```cpp
using DecisionVariant = std::variant<
    RandomActionSystem,
    ReflexActionSystem>;

Action decide(
    DecisionVariant& decision,
    const RandomVacuumAgent& agent,
    const VacuumPercept& percept,
    std::mt19937& generator) {
    return std::visit(
        [&](auto& system) {
            return system(agent, percept, generator);
        },
        decision);
}

std::vector<DecisionVariant> pipeline{
    RandomActionSystem{}, ReflexActionSystem{}
};
```

- ✅ No heap allocation — storage is a fixed-size union sized for the largest alternative.
- ✅ Runtime-swappable among a **known, closed set** of types (unlike function pointers).
- ❌ Must declare every possible system type in the `variant` up front — not open for extension by external code.
- ❌ Slightly more boilerplate (`std::visit` + a visitor lambda) than `std::function`.
- **Best for:** a closed, known set of agent strategies where you want runtime selection without allocation.

### Summary Table

| Approach | Allocation | Captures/state | Runtime-swappable | Extensible by others | Overhead |
|---|---|---|---|---|---|
| `std::function` | Possible (SBO-dependent) | Yes | Yes | Yes | Type erasure indirection |
| Function pointer | None | No | Yes | Yes (if stateless) | Minimal |
| Template | None | Yes | No (compile-time fixed) | Yes (any callable) | None (inlined) |
| `std::variant` | None | Yes | Yes (closed set) | No (must know all types) | `std::visit` dispatch |

### Recommendation for this project

- Start with `std::function` for `DecisionFn`/`WorldTickFn` (Commits 7–9) — it's the simplest to learn and matches the "vector of functions" goal.
- As an optional Commit 12, add a `std::variant`-based version of the pipeline and compare code size/readability against the `std::function` version — this is the most direct educational contrast for an ECS-style closed system set.

---

## Optional Commit 12 — `std::variant` comparison exercise

- [ ] Define `ReflexActionSystem` alongside `RandomActionSystem`.
- [ ] Build `DecisionVariant = std::variant<RandomActionSystem, ReflexActionSystem>`.
- [ ] Rewrite the pipeline using `std::vector<DecisionVariant>` + `std::visit`.
- [ ] Add a test proving both variants produce valid actions.
- [ ] Write a short comment/doc note comparing this to the `std::function` version (readability, allocation, extensibility).


















# Functional / Imperative Refactoring Guide

## Vacuum Agent Learning Project

This document describes a complete refactoring strategy for rewriting the vacuum-agent example in a functional/imperative style.

The design intentionally avoids traditional object-oriented programming semantics:

- No inheritance.
- No base classes.
- No virtual functions.
- No polymorphic ownership.
- No `this`-based behavior.
- No logic inside data structs.
- No `std::shared_ptr` or `std::unique_ptr` for entities.
- No heap allocation for the agent, environment, or random generator.
- Behavior is represented by free functions, functors, lambdas, and function pipelines.
- Data and logic remain separate.
- The design should resemble a small Entity Component System.

`std::function` is encouraged for the system pipeline because this is a learning project exploring higher-order functions and runtime-composable behavior. However, its allocation characteristics should be treated as an explicit design consideration.

---

# 1. Current Design

The current implementation uses an object-oriented hierarchy:

```text
Thing
└── Agent

Environment
└── TrivialVacuumEnvironment
```

The agent owns a program through `std::function`, and the environment stores agents through heap-allocated polymorphic pointers.

The current design also contains several risks:

- `std::shared_ptr` is unnecessary for one agent.
- The environment stores a reference to a random generator.
- Constructor member initialization order caused a Valgrind failure.
- String actions such as `"Left"` and `"Suck"` are not type-safe.
- Generic base classes add complexity that the current example does not need.
- Agent data and agent behavior are coupled together.
- Testing requires understanding ownership and inheritance mechanics.

The new design will represent the simulation as:

```text
World data
    ├── Environment state
    ├── Agent state
    └── Random generator

Systems
    ├── Perception
    ├── Decision
    ├── Action application
    └── Simulation scheduling
```

The data will be stored in plain structs. Functions and function objects will operate on that data.

---

# 2. Target Design

The target design consists of:

```text
Plain data structs
    ├── RandomVacuumAgent
    ├── TrivialVacuumEnvironment
    └── VacuumPercept

Value types
    ├── Location
    ├── Cleanliness
    └── Action

Free functions
    ├── location_index
    ├── cleanliness_at
    ├── set_cleanliness
    ├── perceive
    ├── apply_action
    ├── step
    └── run

Function objects and lambdas
    ├── RandomActionSystem
    ├── WorldTickFn
    └── vector of scheduled systems
```

The simulation flow becomes:

```text
environment + agent
        │
        ▼
    perceive
        │
        ▼
  decision function
        │
        ▼
   Action value
        │
        ▼
  apply_action
        │
        ▼
updated environment + agent
```

---

# 3. Design Principles

## 3.1 Data has no behavior

The primary structs should contain data only.

Avoid:

```cpp
struct Agent {
    void act();
    void move();
    void clean();
};
```

Prefer:

```cpp
struct RandomVacuumAgent {
    Location location = loc_A;
    int performance = 0;
};
```

with behavior represented by free functions:

```cpp
void apply_action(
    TrivialVacuumEnvironment& environment,
    RandomVacuumAgent& agent,
    Action action);
```

## 3.2 Functions operate on explicit state

Functions should receive the data they need as parameters.

Prefer:

```cpp
Action choose_random_action(std::mt19937& generator);
```

over storing the generator as a reference inside an object.

Prefer:

```cpp
void apply_action(
    TrivialVacuumEnvironment& environment,
    RandomVacuumAgent& agent,
    Action action);
```

over:

```cpp
agent.execute_action(action);
```

This makes dependencies visible and avoids hidden object state.

## 3.3 Use immutable parameters where possible

Use `const&` for data that should not be modified:

```cpp
VacuumPercept perceive(
    const TrivialVacuumEnvironment& environment,
    const RandomVacuumAgent& agent);
```

Use non-const references only for state that a function intentionally mutates:

```cpp
void apply_action(
    TrivialVacuumEnvironment& environment,
    RandomVacuumAgent& agent,
    Action action);
```

## 3.4 Use strong types instead of strings

Replace:

```cpp
"Left"
"Right"
"Suck"
```

with:

```cpp
enum class Action {
    Left,
    Right,
    Suck
};
```

This prevents spelling mistakes and makes invalid actions harder to represent.

## 3.5 Keep randomness explicit

Do not hide a random generator inside an agent or environment.

Prefer:

```cpp
Action choose_random_action(std::mt19937& generator);
```

and:

```cpp
run(environment, agent, generator, pipeline, 1000);
```

The generator remains owned by the caller, normally the test.

---

# 4. Ordered Refactoring Checklist

Complete the steps in order. Keep the project compiling and the tests passing after each major phase.

---

## Phase 1: Establish a clean baseline

- [ ] Build the current project.
- [ ] Run the existing Google Test suite.
- [ ] Run the current test under Valgrind.
- [ ] Record the current test executable path.
- [ ] Record the current CMake configuration.
- [ ] Save the current implementation before beginning the refactor.

Useful commands:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

Valgrind:

```bash
valgrind \
  --tool=memcheck \
  --track-origins=yes \
  --leak-check=full \
  --show-leak-kinds=all \
  ./build/tests/unit_tests/aima_unit_tests
```

The baseline is useful because it gives a comparison point for the new design.

---

## Phase 2: Remove the inheritance hierarchy

- [ ] Delete `Thing`.
- [ ] Delete the `Agent` base class.
- [ ] Delete the `Environment` base class.
- [ ] Remove all inheritance declarations.
- [ ] Remove all `virtual` functions.
- [ ] Remove all `override` specifiers.
- [ ] Remove all `final` specifiers.
- [ ] Remove `is_alive()` unless the simulation genuinely needs entity lifetime.
- [ ] Remove `things_`.
- [ ] Remove `agents_`.
- [ ] Remove `std::shared_ptr`.
- [ ] Remove polymorphic ownership.
- [ ] Remove `RandomAgentProgram` in its old class-oriented form.

At the end of this phase, the module should contain no inheritance and no polymorphic classes.

The temporary implementation may be incomplete. The goal is to remove the old architecture before adding the new one.

---

## Phase 3: Define value types

Keep the location type as a value:

```cpp
using Location = std::pair<int, int>;
```

Define the two known locations:

```cpp
inline constexpr Location loc_A{0, 0};
inline constexpr Location loc_B{1, 0};
```

Define cleanliness:

```cpp
enum class Cleanliness {
    Clean,
    Dirty
};
```

Define actions:

```cpp
enum class Action {
    Left,
    Right,
    Suck
};
```

Do not represent actions as strings.

Add a percept value:

```cpp
struct VacuumPercept {
    Location location;
    Cleanliness cleanliness;
};
```

The percept is data returned by the perception function.

---

## Phase 4: Define data-only structs

Create a plain agent-data struct:

```cpp
struct RandomVacuumAgent {
    Location location = loc_A;
    int performance = 0;
};
```

Create a plain environment-data struct:

```cpp
struct TrivialVacuumEnvironment {
    std::array<Cleanliness, 2> status{};
};
```

These structs should contain data only.

They should not contain:

- Constructors with behavior.
- Member functions.
- Virtual functions.
- References to random generators.
- Pointers to other entities.
- Function objects.
- Ownership relationships.

The random generator belongs to the caller and is passed to functions that need randomness.

---

## Phase 5: Add safe location access functions

Implement a location-to-array-index function:

```cpp
[[nodiscard]]
constexpr std::size_t location_index(Location location) {
    if (location == loc_A) {
        return 0;
    }

    if (location == loc_B) {
        return 1;
    }

    throw std::out_of_range{
        "Unknown vacuum-environment location"
    };
}
```

Add a read-only accessor:

```cpp
[[nodiscard]]
Cleanliness cleanliness_at(
    const TrivialVacuumEnvironment& environment,
    Location location) {
    return environment.status.at(location_index(location));
}
```

Add a mutating setter:

```cpp
void set_cleanliness(
    TrivialVacuumEnvironment& environment,
    Location location,
    Cleanliness cleanliness) {
    environment.status.at(location_index(location)) = cleanliness;
}
```

Keep reading and writing separate:

```cpp
cleanliness_at(environment, location);
```

```cpp
set_cleanliness(environment, location, Cleanliness::Clean);
```

Do not use a private non-const overload. There are no classes with access control in this design, and separate function names make intent clearer.

---

## Phase 6: Add deterministic component tests

Before adding random behavior, test the data-access functions.

Example:

```cpp
TEST(EnvironmentTest, ReadsCleanlinessAtLocation) {
    aima::TrivialVacuumEnvironment environment{
        .status{
            aima::Cleanliness::Dirty,
            aima::Cleanliness::Clean
        }
    };

    EXPECT_EQ(
        aima::cleanliness_at(environment, aima::loc_A),
        aima::Cleanliness::Dirty);

    EXPECT_EQ(
        aima::cleanliness_at(environment, aima::loc_B),
        aima::Cleanliness::Clean);
}
```

Test mutation:

```cpp
TEST(EnvironmentTest, SetsCleanlinessAtLocation) {
    aima::TrivialVacuumEnvironment environment{
        .status{
            aima::Cleanliness::Dirty,
            aima::Cleanliness::Clean
        }
    };

    aima::set_cleanliness(
        environment,
        aima::loc_A,
        aima::Cleanliness::Clean);

    EXPECT_EQ(
        aima::cleanliness_at(environment, aima::loc_A),
        aima::Cleanliness::Clean);
}
```

Test invalid locations:

```cpp
TEST(EnvironmentTest, RejectsInvalidLocation) {
    aima::TrivialVacuumEnvironment environment{
        .status{
            aima::Cleanliness::Clean,
            aima::Cleanliness::Clean
        }
    };

    EXPECT_THROW(
        aima::cleanliness_at(environment, {5, 5}),
        std::out_of_range);
}
```

---

## Phase 7: Add an environment factory

Random initialization should be implemented by free functions.

```cpp
[[nodiscard]]
Cleanliness random_cleanliness(std::mt19937& generator) {
    std::uniform_int_distribution<int> distribution{0, 1};

    return distribution(generator) == 0
        ? Cleanliness::Clean
        : Cleanliness::Dirty;
}
```

Create the environment as a value:

```cpp
[[nodiscard]]
TrivialVacuumEnvironment
make_trivial_vacuum_environment(std::mt19937& generator) {
    return TrivialVacuumEnvironment{
        .status{
            random_cleanliness(generator),
            random_cleanliness(generator)
        }
    };
}
```

Important properties:

- The environment does not store the generator.
- The generator remains owned by the caller.
- The returned environment contains only its own state.
- There is no initialization-order dependency involving a generator reference.

Test it with a seeded generator:

```cpp
TEST(EnvironmentTest, FactoryInitializesBothLocations) {
    std::mt19937 generator{9};

    const auto environment =
        aima::make_trivial_vacuum_environment(generator);

    EXPECT_TRUE(
        environment.status[0] == aima::Cleanliness::Clean ||
        environment.status[0] == aima::Cleanliness::Dirty);

    EXPECT_TRUE(
        environment.status[1] == aima::Cleanliness::Clean ||
        environment.status[1] == aima::Cleanliness::Dirty);
}
```

---

## Phase 8: Add perception as a free function

Implement:

```cpp
[[nodiscard]]
VacuumPercept perceive(
    const TrivialVacuumEnvironment& environment,
    const RandomVacuumAgent& agent) {
    return VacuumPercept{
        .location = agent.location,
        .cleanliness =
            cleanliness_at(environment, agent.location)
    };
}
```

Test it:

```cpp
TEST(PerceptionTest, ReturnsAgentLocationAndCurrentCleanliness) {
    const aima::TrivialVacuumEnvironment environment{
        .status{
            aima::Cleanliness::Dirty,
            aima::Cleanliness::Clean
        }
    };

    const aima::RandomVacuumAgent agent{
        .location = aima::loc_A,
        .performance = 0
    };

    const auto percept =
        aima::perceive(environment, agent);

    EXPECT_EQ(percept.location, aima::loc_A);
    EXPECT_EQ(
        percept.cleanliness,
        aima::Cleanliness::Dirty);
}
```

Perception does not mutate either the agent or the environment.

---

## Phase 9: Define decision functions

Create a decision-function signature.

A decision function should receive the current agent, its percept, and the random generator:

```cpp
using DecisionFn = std::function<Action(
    const RandomVacuumAgent&,
    const VacuumPercept&,
    std::mt19937&)>;
```

This is intentionally a higher-order-function design.

It allows multiple decision strategies to share the same interface without inheritance:

```cpp
Random decision function
Reflex decision function
Scripted decision function
Test lambda
```

Implement random action selection as a stateless functor:

```cpp
struct RandomActionSystem {
    Action operator()(
        const RandomVacuumAgent&,
        const VacuumPercept&,
        std::mt19937& generator) const {
        std::uniform_int_distribution<int> distribution{0, 2};

        switch (distribution(generator)) {
        case 0:
            return Action::Left;

        case 1:
            return Action::Right;

        default:
            return Action::Suck;
        }
    }
};
```

The functor contains no state and has no inheritance relationship.

It can be assigned to `DecisionFn`:

```cpp
DecisionFn decision = RandomActionSystem{};
```

A scripted test decision can be represented by a lambda:

```cpp
DecisionFn scripted_decision =
    [](const RandomVacuumAgent&,
       const VacuumPercept&,
       std::mt19937&) {
        return Action::Suck;
    };
```

This is useful for deterministic tests.

---

## Phase 10: Test decision functions

Test that random decisions produce valid actions:

```cpp
TEST(RandomActionSystemTest, ProducesKnownActions) {
    std::mt19937 generator{9};

    const aima::RandomVacuumAgent agent{};
    const aima::VacuumPercept percept{
        .location = aima::loc_A,
        .cleanliness = aima::Cleanliness::Dirty
    };

    const aima::RandomActionSystem decision{};

    for (int i = 0; i < 100; ++i) {
        const auto action =
            decision(agent, percept, generator);

        EXPECT_TRUE(
            action == aima::Action::Left ||
            action == aima::Action::Right ||
            action == aima::Action::Suck);
    }
}
```

Test a deterministic lambda:

```cpp
TEST(DecisionTest, LambdaCanProvideDeterministicAction) {
    aima::DecisionFn decision =
        [](const aima::RandomVacuumAgent&,
           const aima::VacuumPercept&,
           std::mt19937&) {
            return aima::Action::Suck;
        };

    std::mt19937 generator{9};

    const aima::RandomVacuumAgent agent{};
    const aima::VacuumPercept percept{
        .location = aima::loc_A,
        .cleanliness = aima::Cleanliness::Dirty
    };

    EXPECT_EQ(
        decision(agent, percept, generator),
        aima::Action::Suck);
}
```

---

## Phase 11: Implement action application

Move action behavior into a free function:

```cpp
void apply_action(
    TrivialVacuumEnvironment& environment,
    RandomVacuumAgent& agent,
    Action action) {
    switch (action) {
    case Action::Right:
        agent.location = loc_B;
        --agent.performance;
        break;

    case Action::Left:
        agent.location = loc_A;
        --agent.performance;
        break;

    case Action::Suck:
        if (cleanliness_at(environment, agent.location) ==
            Cleanliness::Dirty) {
            agent.performance += 10;
        }

        set_cleanliness(
            environment,
            agent.location,
            Cleanliness::Clean);
        break;
    }
}
```

This is the primary imperative operation: it mutates explicit world data.

---

## Phase 12: Add deterministic action tests

Test cleaning dirty soil:

```cpp
TEST(ActionTest, SuckCleansDirtyLocationAndAwardsPoints) {
    aima::TrivialVacuumEnvironment environment{
        .status{
            aima::Cleanliness::Dirty,
            aima::Cleanliness::Clean
        }
    };

    aima::RandomVacuumAgent agent{
        .location = aima::loc_A,
        .performance = 0
    };

    aima::apply_action(
        environment,
        agent,
        aima::Action::Suck);

    EXPECT_EQ(
        aima::cleanliness_at(environment, aima::loc_A),
        aima::Cleanliness::Clean);

    EXPECT_EQ(agent.performance, 10);
}
```

Test cleaning an already clean location:

```cpp
TEST(ActionTest, SuckOnCleanLocationDoesNotAwardPoints) {
    aima::TrivialVacuumEnvironment environment{
        .status{
            aima::Cleanliness::Clean,
            aima::Cleanliness::Clean
        }
    };

    aima::RandomVacuumAgent agent{
        .location = aima::loc_A,
        .performance = 0
    };

    aima::apply_action(
        environment,
        agent,
        aima::Action::Suck);

    EXPECT_EQ(agent.performance, 0);
}
```

Test movement:

```cpp
TEST(ActionTest, RightMovesAgentToLocationB) {
    aima::TrivialVacuumEnvironment environment{
        .status{
            aima::Cleanliness::Clean,
            aima::Cleanliness::Clean
        }
    };

    aima::RandomVacuumAgent agent{
        .location = aima::loc_A,
        .performance = 0
    };

    aima::apply_action(
        environment,
        agent,
        aima::Action::Right);

    EXPECT_EQ(agent.location, aima::loc_B);
    EXPECT_EQ(agent.performance, -1);
}
```

```cpp
TEST(ActionTest, LeftMovesAgentToLocationA) {
    aima::TrivialVacuumEnvironment environment{
        .status{
            aima::Cleanliness::Clean,
            aima::Cleanliness::Clean
        }
    };

    aima::RandomVacuumAgent agent{
        .location = aima::loc_B,
        .performance = 0
    };

    aima::apply_action(
        environment,
        agent,
        aima::Action::Left);

    EXPECT_EQ(agent.location, aima::loc_A);
    EXPECT_EQ(agent.performance, -1);
}
```

These tests do not require randomness and should be the foundation of the implementation.

---

## Phase 13: Define world tick functions

Define a function type for complete simulation systems:

```cpp
using WorldTickFn = std::function<void(
    TrivialVacuumEnvironment&,
    RandomVacuumAgent&,
    std::mt19937&)>;
```

A world tick function can perform a complete perceive-decide-apply cycle.

Create a factory that turns a decision function into a tick system:

```cpp
[[nodiscard]]
WorldTickFn make_agent_tick(DecisionFn decide) {
    return [
        decide = std::move(decide)
    ](
        TrivialVacuumEnvironment& environment,
        RandomVacuumAgent& agent,
        std::mt19937& generator) {
        const VacuumPercept percept =
            perceive(environment, agent);

        const Action action =
            decide(agent, percept, generator);

        apply_action(environment, agent, action);
    };
}
```

This is a form of function composition:

```text
decision function
        │
        ▼
agent tick function
        │
        ▼
world update
```

No inheritance is required to create different agent strategies.

---

## Phase 14: Define a vector of systems

Create a schedule of systems:

```cpp
using SystemPipeline = std::vector<WorldTickFn>;
```

Example:

```cpp
SystemPipeline pipeline{
    make_agent_tick(RandomActionSystem{})
};
```

The vector establishes deterministic execution order.

Each system is invoked in the order in which it appears:

```cpp
void run(
    TrivialVacuumEnvironment& environment,
    RandomVacuumAgent& agent,
    std::mt19937& generator,
    const SystemPipeline& pipeline,
    int steps) {
    for (int step_number = 0;
         step_number < steps;
         ++step_number) {
        for (const auto& system : pipeline) {
            system(environment, agent, generator);
        }
    }
}
```

This is the beginning of an ECS-style scheduler.

The scheduler knows how to execute systems, but it does not need to know the details of each system.

---

## Phase 15: Test deterministic system ordering

Create two systems that record their execution order:

```cpp
TEST(SystemPipelineTest, RunsSystemsInVectorOrder) {
    std::vector<int> execution_order;

    aima::WorldTickFn first =
        [&execution_order](
            aima::TrivialVacuumEnvironment&,
            aima::RandomVacuumAgent&,
            std::mt19937&) {
            execution_order.push_back(1);
        };

    aima::WorldTickFn second =
        [&execution_order](
            aima::TrivialVacuumEnvironment&,
            aima::RandomVacuumAgent&,
            std::mt19937&) {
            execution_order.push_back(2);
        };

    aima::SystemPipeline pipeline{
        first,
        second
    };

    std::mt19937 generator{9};

    aima::TrivialVacuumEnvironment environment{
        .status{
            aima::Cleanliness::Clean,
            aima::Cleanliness::Clean
        }
    };

    aima::RandomVacuumAgent agent{};

    aima::run(
        environment,
        agent,
        generator,
        pipeline,
        1);

    ASSERT_EQ(execution_order.size(), 2U);
    EXPECT_EQ(execution_order[0], 1);
    EXPECT_EQ(execution_order[1], 2);
}
```

This verifies that the vector is a deterministic system schedule.

---

## Phase 16: Rewrite the RandomVacuumAgent integration test

The integration test should use stack-owned data:

```cpp
#include <gtest/gtest.h>

#include <random>
#include <vector>

import mod_agent;

TEST(RandomVacuumAgentTest, CleansBothLocationsWithinFixedRun) {
    std::mt19937 generator{9};

    aima::TrivialVacuumEnvironment environment =
        aima::make_trivial_vacuum_environment(generator);

    aima::RandomVacuumAgent agent{};

    aima::SystemPipeline pipeline{
        aima::make_agent_tick(
            aima::RandomActionSystem{})
    };

    aima::run(
        environment,
        agent,
        generator,
        pipeline,
        1000);

    EXPECT_EQ(
        aima::cleanliness_at(
            environment,
            aima::loc_A),
        aima::Cleanliness::Clean);

    EXPECT_EQ(
        aima::cleanliness_at(
            environment,
            aima::loc_B),
        aima::Cleanliness::Clean);
}
```

This test contains:

- No inheritance.
- No heap-allocated agent.
- No `std::shared_ptr`.
- No environment-owned generator reference.
- No copied agent.
- No member-function behavior.
- A deterministic random seed.
- A vector-based system schedule.

---

# 5. Allocation Policy

The design has two separate allocation goals.

## 5.1 Entity and world data should not use heap allocation

The following should be stack/value objects:

```cpp
std::mt19937 generator{9};
aima::RandomVacuumAgent agent{};
aima::TrivialVacuumEnvironment environment{};
```

There should be no:

```cpp
new
delete
std::shared_ptr
std::unique_ptr
```

for these objects.

## 5.2 `std::function` and `std::vector` may allocate internally

`std::function` and `std::vector` are standard library containers. They may allocate dynamically.

For this learning project, that allocation is acceptable if the goal is to explore:

- Higher-order functions.
- Runtime-composable systems.
- Lambdas.
- Functors.
- Function pipelines.
- Deterministic system scheduling.

The important distinction is:

```text
No heap allocation for entities/components/world state
```

versus:

```text
No heap allocation anywhere in the program
```

Those are different requirements.

If strict zero-allocation execution becomes a future goal, replace:

```cpp
std::vector<WorldTickFn>
std::function<...>
```

with alternatives such as:

- `std::array` of function pointers.
- `std::span` over a fixed system array.
- Stateless function objects passed as templates.
- `std::variant` of known system types.
- A custom non-owning callable wrapper.
- Compile-time system composition.

For the current project, `std::function` is useful and educational.

---

# 6. Optional Strict Zero-Allocation Variant

If strict zero allocation is required for the pipeline itself, use function pointers for stateless systems:

```cpp
using WorldTickFn = void(*)(
    TrivialVacuumEnvironment&,
    RandomVacuumAgent&,
    std::mt19937&);
```

Then use a fixed-size array:

```cpp
using SystemPipeline =
    std::array<WorldTickFn, 1>;
```

This prevents captured lambdas and stateful functors, so it is less flexible.

Another option is compile-time composition:

```cpp
template <typename... Systems>
void run(
    TrivialVacuumEnvironment& environment,
    RandomVacuumAgent& agent,
    std::mt19937& generator,
    int steps,
    Systems... systems);
```

However, this changes the learning objective from runtime function composition to template-based static composition.

For this project, begin with `std::function` and document its allocation tradeoff.

---

# 7. Suggested Module API

The final module should expose a small data-and-function API similar to this:

```cpp
export namespace aima {

using Location = std::pair<int, int>;

inline constexpr Location loc_A{0, 0};
inline constexpr Location loc_B{1, 0};

enum class Cleanliness {
    Clean,
    Dirty
};

enum class Action {
    Left,
    Right,
    Suck
};

struct VacuumPercept {
    Location location;
    Cleanliness cleanliness;
};

struct RandomVacuumAgent {
    Location location = loc_A;
    int performance = 0;
};

struct TrivialVacuumEnvironment {
    std::array<Cleanliness, 2> status{};
};

using DecisionFn = std::function<Action(
    const RandomVacuumAgent&,
    const VacuumPercept&,
    std::mt19937&)>;

using WorldTickFn = std::function<void(
    TrivialVacuumEnvironment&,
    RandomVacuumAgent&,
    std::mt19937&)>;

using SystemPipeline = std::vector<WorldTickFn>;

struct RandomActionSystem {
    Action operator()(
        const RandomVacuumAgent& agent,
        const VacuumPercept& percept,
        std::mt19937& generator) const;
};

[[nodiscard]]
std::size_t location_index(Location location);

[[nodiscard]]
Cleanliness cleanliness_at(
    const TrivialVacuumEnvironment& environment,
    Location location);

void set_cleanliness(
    TrivialVacuumEnvironment& environment,
    Location location,
    Cleanliness cleanliness);

[[nodiscard]]
Cleanliness random_cleanliness(
    std::mt19937& generator);

[[nodiscard]]
TrivialVacuumEnvironment
make_trivial_vacuum_environment(
    std::mt19937& generator);

[[nodiscard]]
VacuumPercept perceive(
    const TrivialVacuumEnvironment& environment,
    const RandomVacuumAgent& agent);

void apply_action(
    TrivialVacuumEnvironment& environment,
    RandomVacuumAgent& agent,
    Action action);

[[nodiscard]]
WorldTickFn make_agent_tick(
    DecisionFn decide);

void run(
    TrivialVacuumEnvironment& environment,
    RandomVacuumAgent& agent,
    std::mt19937& generator,
    const SystemPipeline& pipeline,
    int steps);

}  // namespace aima
```

The structs are data-only. The functions and functors contain the behavior.

---

# 8. Optional Extensions

After the random-agent refactor is complete, add more systems without changing the core data model.

## 8.1 Reflex decision function

```cpp
aima::DecisionFn reflex_decision =
    [](
        const aima::RandomVacuumAgent&,
        const aima::VacuumPercept& percept,
        std::mt19937&) {
        if (percept.cleanliness ==
            aima::Cleanliness::Dirty) {
            return aima::Action::Suck;
        }

        if (percept.location == aima::loc_A) {
            return aima::Action::Right;
        }

        return aima::Action::Left;
    };
```

No subclass is needed.

## 8.2 Scripted decision function

```cpp
std::vector<aima::Action> script{
    aima::Action::Suck,
    aima::Action::Right,
    aima::Action::Suck
};
```

A lambda can consume the script deterministically:

```cpp
std::size_t index = 0;

aima::DecisionFn scripted_decision =
    [&script, &index](
        const aima::RandomVacuumAgent&,
        const aima::VacuumPercept&,
        std::mt19937&) {
        return script.at(index++);
    };
```

This is useful for deterministic integration tests.

## 8.3 Logging system

Add a system that observes the world:

```cpp
aima::WorldTickFn logging_system =
    [](
        aima::TrivialVacuumEnvironment&,
        aima::RandomVacuumAgent& agent,
        std::mt19937&) {
        std::cout
            << "Location: "
            << agent.location.first
            << ", "
            << agent.location.second
            << '\n';
    };
```

The logging system can be placed before or after the agent system in the pipeline.

## 8.4 Multiple agents

If the project later needs multiple agents, introduce a data container:

```cpp
std::vector<RandomVacuumAgent> agents;
```

Keep the agents as values. Do not immediately reintroduce an `Agent` base class.

Systems can iterate over the collection:

```cpp
for (auto& agent : agents) {
    // apply perception, decision, and action
}
```

This remains compatible with an ECS-inspired design.

---

# 9. Final Verification Checklist

- [ ] No class inherits from another class.
- [ ] No `virtual` functions remain.
- [ ] No `override` or `final` specifiers remain.
- [ ] No `Thing`, `Agent`, or `Environment` base classes remain.
- [ ] The agent is a plain data struct.
- [ ] The environment is a plain data struct.
- [ ] No entity uses `new`.
- [ ] No entity uses `std::shared_ptr`.
- [ ] No entity uses `std::unique_ptr`.
- [ ] The random generator is owned by the caller.
- [ ] No environment member stores a generator reference.
- [ ] Actions use `enum class`.
- [ ] Perception is a free function.
- [ ] Action application is a free function.
- [ ] Random decisions are implemented by a functor or lambda.
- [ ] Systems are stored in a deterministic vector.
- [ ] System execution order is tested.
- [ ] Deterministic action tests exist.
- [ ] The random integration test uses a fixed seed.
- [ ] The random integration test uses stack-owned state.
- [ ] `std::function` allocation tradeoffs are documented.
- [ ] The full test suite passes.
- [ ] Valgrind reports no invalid reads or writes.

Build:

```bash
cmake --build build
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

Run Valgrind:

```bash
valgrind \
  --tool=memcheck \
  --track-origins=yes \
  --leak-check=full \
  --show-leak-kinds=all \
  ./build/tests/unit_tests/aima_unit_tests
```

Expected final result:

```text
ERROR SUMMARY: 0 errors from 0 contexts
```

---

# 10. End State

The completed design should have the following characteristics:

```text
Data:
    Plain structs
    Value semantics
    Stack ownership
    No inheritance

Logic:
    Free functions
    Stateless functors
    Lambdas
    std::function-based interfaces

Scheduling:
    Ordered vector of systems
    Explicit execution order
    Deterministic tests

Randomness:
    Generator owned by caller
    Generator passed explicitly
    No dangling references

Testing:
    Deterministic unit tests for rules
    Seeded integration test for random behavior
    Valgrind verification
```

The important architectural lesson is that polymorphism is not the only way to vary behavior. In this design, behavior varies through values representing functions:

```text
inheritance and virtual dispatch
        becomes
functors, lambdas, std::function, and system pipelines
```

The result is a small, explicit, data-oriented program that demonstrates functional and imperative techniques without requiring traditional OOP semantics.











