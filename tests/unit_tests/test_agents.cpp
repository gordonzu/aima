#include <gtest/gtest.h> 
#include <sstream>

import aima.environment;
import aima.agent;

using namespace environment;

TEST(XYEnvironmentTest, ConstructMatrix) 
{
    auto env = makeXYEnvironment(10, 10); 
    ASSERT_EQ(mapSize(env), size_t(100));
}

TEST(XyEnvironmentTest, AddAgentToLocation) 
{
    auto env    = makeXYEnvironment(12, 10);
    auto loc    = XYLocation(3, 4);
    auto agent  = XYAgent();
    std::ostringstream oss;

    ASSERT_TRUE(addAgentToLocation(agent, env, loc));

    oss << getObjectAt(env, loc);

    ASSERT_EQ(oss.str(), "[xyagent, 1]");
}


