#include <gtest/gtest.h>
#include <random>

import mod_agent;

TEST(RandomVacuumAgentTest, CleanBothLocations) {
    std::mt19937 generator{9};

    aima::RandomVacuumAgent agent{generator};
    aima::TrivialVacuumEnvironment env{generator};

    env.add_thing(std::make_shared<aima::RandomVacuumAgent>(agent));
    env.run();

    EXPECT_EQ(env.cleanliness_at(aima::loc_A), aima::Cleanliness::Clean);
    EXPECT_EQ(env.cleanliness_at(aima::loc_B), aima::Cleanliness::Clean); 
}


