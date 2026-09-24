module;

#include <string>
#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <random>
#include <utility>
#include <vector>

export module ModAgent;

namespace aima::agent {

using Location = std::pair<int, int>;

export inline constexpr Location loc_A{0, 0};
export inline constexpr Location loc_B{1, 0};

export enum class Cleanliness {
    Clean,
    Dirty
};

export struct VacuumPercept {};

} // ModAgent
