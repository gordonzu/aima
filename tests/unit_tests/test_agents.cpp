#include <gtest/gtest.h> 
#include <sstream>

import aima.environment;
import aima.agent;

using namespace environment;

TEST(XYEnvironmentTest, ConstructMatrix) {
    auto env = makeXYEnvironment(10, 10); 
    ASSERT_EQ(mapSize(env), size_t(100));
}

TEST(XYEnvironmentTest, EmptyLocationReturnsNone) {
    auto env = makeXYEnvironment(4, 4);
    ASSERT_FALSE(getObjectAt(env, XYLocation(2, 2)).has_value());
    ASSERT_FALSE(isOccupied(env, XYLocation(2, 2)));
}

TEST(XYEnvironmentTest, AddToOccupiedLocationFails) {
    auto env = makeXYEnvironment(4, 4);
    auto loc = XYLocation(2, 2);
    ASSERT_TRUE(addAgentToLocation(XYAgent(), env, loc));
    ASSERT_FALSE(addAgentToLocation(XYAgent(), env, loc));
}

TEST(XyEnvironmentTest, AddOutOfBoundsFails) {
    auto env = makeXYEnvironment(4, 4);
    ASSERT_FALSE(addAgentToLocation(XYAgent(), env, XYLocation(5, 1)))  ;
}

TEST(XyEnvironmentTest, AddAgentToLocation) {
    auto env    = makeXYEnvironment(12, 10);
    auto loc    = XYLocation(3, 4);
    auto agent  = XYAgent();
    std::ostringstream oss, expected;

    ASSERT_TRUE(addAgentToLocation(agent, env, loc));

    oss << getObjectAt(env, loc);
    expected << agent.ref_;

    ASSERT_EQ(oss.str(), expected.str());
}

TEST(XyEnvironmentTest, IsOccupiedAfterAdd) {
    auto env = makeXYEnvironment(4, 4);
    auto loc = XYLocation(1, 1);
    addAgentToLocation(XYAgent(), env, loc);

    ASSERT_TRUE(isOccupied(env, loc));
}

TEST(XyEnvironmentTest, AgentIsRegisteredAfterAdd) {
    auto env = makeXYEnvironment(12, 10);
    auto loc = XYLocation(6, 6);
    auto agent = XYAgent();

    addAgentToLocation(agent, env, loc);
    ASSERT_EQ(1, (int)env.agents_.size());
    ASSERT_TRUE(hasAgent(env.agents_, agent));
}




























