#include <gtest/gtest.h> 
#include <sys/ucontext.h>

import aima.environment;
import aima.agent;

using namespace environment;

class XYEnvironmentTest : public testing::Test {
protected:
    XYEnvironment env = makeXYEnvironment(4, 4);
};

class XYEnvironmentAgentTest : public testing::Test {
protected:
    XYEnvironmentAgentTest() : env(makeXYEnvironment(4, 4)), 
                               source(2, 2),
                               destination(3, 3) {}

    XYEnvironment env;
    XYLocation source;
    XYLocation destination;
};

TEST_F(XYEnvironmentTest, ConstructXYEnvironmentGrid) {
    auto grid = makeXYEnvironment(3, 5);

    EXPECT_EQ(mapSize(grid), size_t(15));
    EXPECT_TRUE(inBounds(grid, XYLocation(1, 1)));
    EXPECT_TRUE(inBounds(grid, XYLocation(3, 5)));
}

TEST_F(XYEnvironmentTest, RejectsNonPositiveDimensions) {
    EXPECT_THROW(makeXYEnvironment(0, 4), std::invalid_argument);
    EXPECT_THROW(makeXYEnvironment(4, 0), std::invalid_argument);
    EXPECT_THROW(makeXYEnvironment(-1, 4), std::invalid_argument);
    EXPECT_THROW(makeXYEnvironment(4, -1), std::invalid_argument);
}

TEST_F(XYEnvironmentTest, BoundsAreOneBasedAndInclusive) {
    EXPECT_TRUE(inBounds(env, XYLocation(1, 1)));
    EXPECT_TRUE(inBounds(env, XYLocation(4, 4)));
    EXPECT_FALSE(inBounds(env, XYLocation(0, 1)));
    EXPECT_FALSE(inBounds(env, XYLocation(1, 0)));
    EXPECT_FALSE(inBounds(env, XYLocation(-1, 1)));
    EXPECT_FALSE(inBounds(env, XYLocation(1, -1)));
    EXPECT_FALSE(inBounds(env, XYLocation(5, 1)));
    EXPECT_FALSE(inBounds(env, XYLocation(1, 5)));
}

TEST_F(XYEnvironmentTest, EmptyAndOccupiedQueriesAgree) {
    const XYLocation loc{2, 2};
    XYAgent agent;

    EXPECT_FALSE(getObjectAt(env, loc).has_value());
    EXPECT_FALSE(isOccupied(env, loc));

    ASSERT_TRUE(addObjectTo(agent, env, loc));
    EXPECT_TRUE(getObjectAt(env, loc).has_value());
    EXPECT_TRUE(isOccupied(env, loc));
}

TEST_F(XYEnvironmentTest, OutOfBoundsQueriesReturnNoObjectAndAreNotOccupied) {
    for (const XYLocation loc : {XYLocation(0, 1), XYLocation(1, 0),
                                 XYLocation(5, 1), XYLocation(1, 5)}) {
        EXPECT_FALSE(getObjectAt(env, loc).has_value());
        EXPECT_FALSE(isOccupied(env, loc));
    }
}

TEST_F(XYEnvironmentAgentTest, AddRegistersAgentAndRecordsLocation) {
    XYAgent agent;

    ASSERT_TRUE(addObjectTo(agent, env, source));

    EXPECT_TRUE(agentIsRegistered(env.agents_, agent));
    ASSERT_TRUE(getLocationFor(env, agent.ref_).has_value());
    EXPECT_EQ(getLocationFor(env, agent.ref_).value(), source);
    EXPECT_TRUE(isOccupied(env, source));

    const auto object = getObjectAt(env, source);
    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(object->type_, Type::xyagent);
    EXPECT_EQ(object->id_, agent.ref_.id_);
}

TEST_F(XYEnvironmentAgentTest, AddRejectsOccupiedLocationWithoutChangingState) {
    XYAgent first;
    XYAgent second;

    ASSERT_TRUE(addObjectTo(first, env, source));
    EXPECT_FALSE(addObjectTo(second, env, source));

    EXPECT_TRUE(agentIsRegistered(env.agents_, first));
    EXPECT_FALSE(agentIsRegistered(env.agents_, second));
    ASSERT_TRUE(getLocationFor(env, first.ref_).has_value());
    EXPECT_EQ(getLocationFor(env, first.ref_).value(), source);
    EXPECT_FALSE(getLocationFor(env, second.ref_).has_value());
    EXPECT_FALSE(isOccupied(env, destination));
    
    const auto object = getObjectAt(env, source);
    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(object->id_, first.ref_.id_);
}

TEST_F(XYEnvironmentAgentTest, AddRejectsAlreadyRegisteredAgentAtAnotherLocation) {
    XYAgent agent;

    ASSERT_TRUE(addObjectTo(agent, env, source));
    EXPECT_FALSE(addObjectTo(agent, env, destination));

    ASSERT_TRUE(getLocationFor(env, agent.ref_).has_value());
    EXPECT_EQ(getLocationFor(env, agent.ref_).value(), source);
    EXPECT_TRUE(isOccupied(env, source));
    EXPECT_FALSE(isOccupied(env, destination));
}

TEST_F(XYEnvironmentAgentTest, AddRejectsOutOfBoundsLocationWithoutRegisteringAgent) {
    XYAgent agent;

    EXPECT_FALSE(addObjectTo(agent, env, XYLocation(5, 1)));
    EXPECT_FALSE(agentIsRegistered(env.agents_, agent));
    EXPECT_FALSE(getLocationFor(env, agent.ref_).has_value());
}

TEST_F(XYEnvironmentAgentTest, AddAcceptsBoundaryLocations) {
    XYAgent first;
    XYAgent last;

    EXPECT_TRUE(addObjectTo(first, env, XYLocation(1, 1)));
    EXPECT_TRUE(addObjectTo(last, env, XYLocation(4, 4)));
}

TEST_F(XYEnvironmentAgentTest, MoveToEmptyLocationVacatesSource) {
    XYAgent agent;

    ASSERT_TRUE(addObjectTo(agent, env, source));
    ASSERT_TRUE(moveObjectTo(agent.ref_, env, destination));

    EXPECT_FALSE(isOccupied(env, source));
    EXPECT_TRUE(isOccupied(env, destination));
    ASSERT_TRUE(getLocationFor(env, agent.ref_).has_value());
    EXPECT_EQ(getLocationFor(env, agent.ref_).value(), destination);

    const auto object = getObjectAt(env, destination);
    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(object->id_, agent.ref_.id_);
} 

TEST_F(XYEnvironmentAgentTest, MoveRejectsOccupiedDestinationWithoutChangingState) {
    XYAgent moving;
    XYAgent stationary;

    ASSERT_TRUE(addObjectTo(moving, env, source));
    ASSERT_TRUE(addObjectTo(stationary, env, destination));

    EXPECT_FALSE(moveObjectTo(moving.ref_, env, destination));

    EXPECT_TRUE(isOccupied(env, source));
    EXPECT_TRUE(isOccupied(env, destination));
    ASSERT_TRUE(getLocationFor(env, moving.ref_).has_value());
    EXPECT_EQ(getLocationFor(env, moving.ref_).value(), source);

    const auto sourceObject = getObjectAt(env, source);
    const auto destinationObject = getObjectAt(env, destination);

    ASSERT_TRUE(sourceObject.has_value());
    ASSERT_TRUE(destinationObject.has_value());
    EXPECT_EQ(sourceObject->id_, moving.ref_.id_);
    EXPECT_EQ(destinationObject->id_, stationary.ref_.id_);
}

TEST_F(XYEnvironmentAgentTest, MoveRejectsOutOfBoundsDestinationWithoutChangingState) {
    XYAgent agent;

    ASSERT_TRUE(addObjectTo(agent, env, source));
    EXPECT_FALSE(moveObjectTo(agent.ref_, env, XYLocation(5, 1)));

    EXPECT_TRUE(isOccupied(env, source));
    ASSERT_TRUE(getLocationFor(env, agent.ref_).has_value());
    EXPECT_EQ(getLocationFor(env, agent.ref_).value(), source);
}

TEST_F(XYEnvironmentAgentTest, MoveRejectsUnregisteredAgent) {
    XYAgent agent;

    EXPECT_FALSE(moveObjectTo(agent.ref_, env, destination));

    EXPECT_FALSE(agentIsRegistered(env.agents_, agent));
    EXPECT_FALSE(getLocationFor(env, agent.ref_).has_value());
    EXPECT_FALSE(isOccupied(env, destination));
}

TEST_F(XYEnvironmentAgentTest, MoveToCurrentLocationFailsWithoutChangingState) {
    XYAgent agent;

    ASSERT_TRUE(addObjectTo(agent, env, source));
    EXPECT_FALSE(moveObjectTo(agent.ref_, env, source));

    EXPECT_TRUE(isOccupied(env, source));
    EXPECT_FALSE(isOccupied(env, destination));
    ASSERT_TRUE(getLocationFor(env, agent.ref_).has_value());
    EXPECT_EQ(getLocationFor(env, agent.ref_).value(), source);
}




















/*
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
    auto env = makeXYEnvironment(12, 10);
    XYLocation loc(3, 4);
    XYAgent agent;
    std::ostringstream oss, expected;

    ASSERT_TRUE(addObjectTo(agent, env, loc));
    oss << getObjectAt(env, loc);
    expected << agent.ref_;
    ASSERT_EQ(oss.str(), expected.str());
}

TEST(XYEnvironmentTest, AgentIsRegisteredAfterAdd) {
    auto env = makeXYEnvironment(12, 10);
    XYLocation loc(6, 6);
    XYAgent agent;

    ASSERT_TRUE(addObjectTo(agent, env, loc));
    ASSERT_TRUE(agentIsRegistered(env.agents_, agent));
}

TEST(XYEnvironmentTest, LocationIsOccupiedAfterAdd) { 
    auto env = makeXYEnvironment(4, 4); 
    XYLocation loc(1, 1); 
    XYAgent agent; 

    ASSERT_TRUE(addObjectTo(agent, env, loc)); 
    ASSERT_TRUE(isOccupied(env, loc)); 
} 

TEST(XYEnvironmentTest, AddAgentTwiceToSameLocationFails) { 
    auto env = makeXYEnvironment(4, 4); 
    XYLocation loc(2,2); 
    XYAgent agent; 

    ASSERT_TRUE(addObjectTo(agent, env, loc)); 
    ASSERT_FALSE(addObjectTo(agent, env, loc)); 
} 

TEST(XYEnvironmentTest, AddOutOfBoundsFails) { 
    auto env = makeXYEnvironment(4, 4); 

    ASSERT_FALSE(addObjectTo(XYAgent(), env, XYLocation(5,1))); 
}

// should agent registration happen on agent creation or when agent is added to a location????

TEST(XYEnvironmentTest, AgentCanMoveToAbsoluteLocation) {
    auto env = makeXYEnvironment(12, 10);
    XYAgent agent;
    XYLocation loc1(2, 2);
    XYLocation loc2(6, 7);

    addObjectTo(agent, env, loc1);
    ASSERT_EQ(loc1, getLocationFor(env, agent.ref_));

    moveObjectTo(agent, env, loc2);
    ASSERT_EQ(loc2, getLocationFor(env, agent.ref_));
}
*/















