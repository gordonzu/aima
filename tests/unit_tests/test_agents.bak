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
    ASSERT_FALSE(getObjectAtLocation(env, XYLocation(2, 2)).has_value());
    ASSERT_FALSE(isOccupied(env, XYLocation(2, 2)));
}

TEST(XYEnvironmentTest, AddAgentToLocation) {
    auto env = makeXYEnvironment(12, 10);
    XYLocation loc(3, 4);
    XYAgent agent;
    std::ostringstream oss, expected;

    ASSERT_TRUE(addObjectToLocation(agent, env, loc));
    oss << getObjectAtLocation(env, loc);
    expected << agent.ref_;
    ASSERT_EQ(oss.str(), expected.str());
}

TEST(XYEnvironmentTest, AgentIsRegisteredAfterAdd) {
    auto env = makeXYEnvironment(12, 10);
    XYLocation loc(6, 6);
    XYAgent agent;

    ASSERT_TRUE(addObjectToLocation(agent, env, loc));
    ASSERT_TRUE(agentIsRegistered(env.agents_, agent));
}

TEST(XYEnvironmentTest, LocationIsOccupiedAfterAdd) { 
    auto env = makeXYEnvironment(4, 4); 
    XYLocation loc(1, 1); 
    XYAgent agent; 

    ASSERT_TRUE(addObjectToLocation(agent, env, loc)); 
    ASSERT_TRUE(isOccupied(env, loc)); 
} 

TEST(XYEnvironmentTest, AddAgentTwiceToSameLocationFails) { 
    auto env = makeXYEnvironment(4, 4); 
    XYLocation loc(2,2); 
    XYAgent agent; 

    ASSERT_TRUE(addObjectToLocation(agent, env, loc)); 
    ASSERT_FALSE(addObjectToLocation(agent, env, loc)); 
} 

TEST(XYEnvironmentTest, AddOutOfBoundsFails) { 
    auto env = makeXYEnvironment(4, 4); 

    ASSERT_FALSE(addObjectToLocation(XYAgent(), env, XYLocation(5,1))); 
}

// should agent registration happen on agent creation or when agent is added to a location????

TEST(XYEnvironmentTest, AgentCanMoveToAbsoluteLocation) {
    auto env = makeXYEnvironment(12, 10);
    XYAgent agent;
    XYLocation loc1(2, 2);
    XYLocation loc2(6, 7);

    addObjectToLocation(agent, env, loc1);
    ASSERT_EQ(loc1, getLocationForObject(env, agent.ref_));

    moveObjectToLocation(agent, env, loc2);
    ASSERT_EQ(loc2, getLocationForObject(env, agent.ref_));
}
















