module;

#include <string>
#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <random>
#include <utility>
#include <vector>

export module mod_agent;

namespace aima {

using Location = std::pair<int, int>;

export inline constexpr Location loc_A{0, 0};
export inline constexpr Location loc_B{1, 0};

export enum class Cleanliness {
    Clean,
    Dirty
};

export class Thing {
public:
    virtual ~Thing() = default;
    virtual bool is_alive() const { return alive_; }

protected:
    bool alive_ = false;
};

export class Agent : public Thing {
public:
    using Program = std::function<std::string(const std::pair<Location, Cleanliness>&)>;
    
    explicit Agent(Program program = {})
        : program_(std::move(program)) {
        alive_ = true;
    }

    std::string act(const std::pair<Location, Cleanliness>& percept) const {
        return program_ ? program_(percept) : "";
    }

    Location location{};
    bool bump = false;
    int performance = 0;

private:
    Program program_;
    bool alive_ = false;
};

export class Environment {
public:
    virtual ~Environment() = default;
    virtual std::pair<Location, Cleanliness> percept(const Agent& agent) const = 0;
    virtual void execute_action(Agent& agent, const std::string& action) = 0;

    void add_thing(const std::shared_ptr<Agent>& agent) {
        agent->location = default_location();
        agent->performance = 0;
        things_.push_back(agent);
        agents_.push_back(agent);
    }

    bool is_done() const {
        return std::none_of(
            agents_.begin(), agents_.end(),
            [](const auto& agent) { return agent->is_alive(); });
    }

    void step() {
        if (is_done()) {
            return;
        }

        std::vector<std::string> actions;
        actions.reserve(agents_.size());

        for (const auto& agent : agents_) {
            actions.push_back(
                agent->is_alive() ? agent->act(percept(*agent)) : "");
        }

        for (std::size_t i = 0; i < agents_.size(); ++i) {
            execute_action(*agents_[i], actions[i]);
        }
    }

    void run(int steps = 1000) {
        for (int step = 0; step < steps && !is_done(); ++step) {
            this->step();
        }
    }

protected:
    virtual Location default_location() = 0;
    std::vector<std::shared_ptr<Thing>> things_;
    std::vector<std::shared_ptr<Agent>> agents_;
};

export class TrivialVacuumEnvironment final : public Environment {
    Location default_location() override {
        std::uniform_int_distribution<int> distribution(0, 1);
        return distribution(generator_) == 0 ? loc_A : loc_B;
    }

    Cleanliness random_cleanliness() {
        std::uniform_int_distribution<int> distribution(0, 1);
        return distribution(generator_) == 0
            ? Cleanliness::Clean
            : Cleanliness::Dirty;
    }

   void set_cleanliness(const Location& location, Cleanliness c) {
        status.at(index_for(location)) = c;
    }

    static std::size_t index_for(const Location& location) {
        return location == loc_A ? 0U : 1U;
    }

    std::mt19937& generator_;

public:
    explicit TrivialVacuumEnvironment(std::mt19937& generator)
        : generator_(generator), status{ random_cleanliness(), random_cleanliness(), } {}

    std::pair<Location, Cleanliness> percept(const Agent& agent) const override {
        return {agent.location, cleanliness_at(agent.location)};
    }

    void execute_action(Agent& agent, const std::string& action) override {
        if (action == "Right") {
            agent.location = loc_B;
            --agent.performance;
        } else if (action == "Left") {
            agent.location = loc_A;
            --agent.performance;
        } else if (action == "Suck") {
            if (cleanliness_at(agent.location) == Cleanliness::Dirty) {
                agent.performance += 10;
            }
            set_cleanliness(agent.location, Cleanliness::Clean);
        }
    }

    Cleanliness cleanliness_at(const Location& location) const {
        return status.at(index_for(location));
    }

    std::array<Cleanliness, 2> status;

};

inline Agent::Program RandomAgentProgram(std::mt19937& generator) {
    return [&generator](const std::pair<Location, Cleanliness>&) {
        static constexpr std::array<const char*, 3> actions{
            "Right", "Left", "Suck"
        };

        std::uniform_int_distribution<std::size_t> distribution(
            0, actions.size() - 1);

        return std::string{actions[distribution(generator)]};
    };
}

export class RandomVacuumAgent final : public Agent {
public:
    explicit RandomVacuumAgent(std::mt19937& generator)
        : Agent(RandomAgentProgram(generator)) {}
};

}























