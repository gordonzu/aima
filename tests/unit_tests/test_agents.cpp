#include <gtest/gtest.h> 

import aima.environment;
import aima.agent;

using namespace env;

TEST(XYEnvironmentTest, ConstructMatrix) 
{
    auto env = makeXYEnvironment(10, 10); 
    ASSERT_EQ(mapSize(env), size_t(100));
}

TEST(XyEnvironmentTest, AddAgentToLocation) 
{
    auto env = makeXYEnvironment(10, 12);
    auto xy = XYLocation(3, 4);
    auto agent = XYAgent();
    
    ASSERT_TRUE(addAgentToLocation(agent, env, xy));
}


