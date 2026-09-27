#include <gtest/gtest.h> 

import aima.environment;
import aima.agent;

using namespace xyenv;

TEST(XYEnvironmentTest, ConstructMatrix) {
    auto x = make_xy_environment(10, 10); 
    ASSERT_EQ(getMapSize(x), size_t(100));
}


