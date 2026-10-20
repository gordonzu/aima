#include <gtest/gtest.h> 
#include <sstream>

import aima.environment;
import aima.agent;

using namespace environment;

TEST(XYEnvironmentTest, ConstructMatrix) {
    auto env = makeXYEnvironment(10, 10); 
    ASSERT_EQ(mapSize(env), size_t(100));
}

TEST(XYEnvironmentTest, EmptyLocationIsUnoccupied) {
    auto env = makeXYEnvironment(4, 4);
    ASSERT_FALSE(getObjectAt(env, XYLocation(2, 2)).has_value());
    ASSERT_FALSE(isOccupied(env, XYLocation(2, 2)));
}

TEST(XYEnvironmentTest, AddAgentToLocation) {
    auto env    = makeXYEnvironment(12, 10);
    auto loc    = XYLocation(3, 4);
    auto agent  = XYAgent();
    std::ostringstream oss, expected;

    registerAgent(env.agents_, agent);
    ASSERT_TRUE(addAgentToLocation(agent, env, loc));
    oss << getObjectAt(env, loc);
    expected << agent.ref_;
    ASSERT_EQ(oss.str(), expected.str());
}

TEST(XYEnvironmentTest, LocationIsOccupiedAfterAdd) {
    auto env = makeXYEnvironment(4, 4);
    auto loc = XYLocation(1, 1);
    auto agent = XYAgent();
    registerAgent(env.agents_, agent);

    ASSERT_TRUE(addAgentToLocation(agent, env, loc));
    ASSERT_TRUE(isOccupied(env, loc));
}

TEST(XYEnvironmentTest, UnregisteredAgentFailsToAdd) {
    auto env = makeXYEnvironment(12, 10);
    auto loc = XYLocation(6, 6);
    auto agent = XYAgent();

    ASSERT_FALSE(addAgentToLocation(agent, env, loc));
}

TEST(XYEnvironmentTest, AddAgentTwiceFails) {
    auto env = makeXYEnvironment(4, 4);
    auto loc = XYLocation(2, 2);
    auto agent = XYAgent();
    registerAgent(env.agents_, agent);

    ASSERT_TRUE(addAgentToLocation(agent, env, loc));
    ASSERT_FALSE(addAgentToLocation(agent, env, loc));
}

TEST(XYEnvironmentTest, AddOutOfBoundsFails) {
    auto env = makeXYEnvironment(4, 4);
    ASSERT_FALSE(addAgentToLocation(XYAgent(), env, XYLocation(5, 1)))  ;
}
























