#include <gtest/gtest.h>

#include <stdexcept>

import aima.environment;

using namespace environment;

class XYEnvironmentTest : public testing::Test {
protected:
    XYEnvironment env = makeXYEnvironment(4, 4);
};

class XYEnvironmentAgentTest : public testing::Test {
protected:
    XYEnvironment env = makeXYEnvironment(4, 4);
    const XYLocation source{2, 2};
    const XYLocation destination{3, 3};
};

TEST_F(XYEnvironmentTest, ConstructsRectangularGrid) {
    auto rectangular = makeXYEnvironment(3, 5);

    EXPECT_EQ(mapSize(rectangular), size_t(15));
    EXPECT_TRUE(inBounds(rectangular, XYLocation(1, 1)));
    EXPECT_TRUE(inBounds(rectangular, XYLocation(3, 5)));
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

    EXPECT_FALSE(getObjectAtLocation(env, loc).has_value());
    EXPECT_FALSE(isOccupied(env, loc));

    ASSERT_TRUE(addObjectToLocation(agent, env, loc));
    EXPECT_TRUE(getObjectAtLocation(env, loc).has_value());
    EXPECT_TRUE(isOccupied(env, loc));
}

TEST_F(XYEnvironmentTest, OutOfBoundsQueriesReturnNoObjectAndAreNotOccupied) {
    for (const XYLocation loc : {XYLocation(0, 1), XYLocation(1, 0),
                                 XYLocation(5, 1), XYLocation(1, 5)}) {
        EXPECT_FALSE(getObjectAtLocation(env, loc).has_value());
        EXPECT_FALSE(isOccupied(env, loc));
    }
}

TEST_F(XYEnvironmentAgentTest, AddRegistersAgentAndRecordsLocation) {
    XYAgent agent;

    ASSERT_TRUE(addObjectToLocation(agent, env, source));

    EXPECT_TRUE(agentIsRegistered(env.agents_, agent));
    ASSERT_TRUE(getLocationForObject(env, agent.ref_).has_value());
    EXPECT_EQ(getLocationForObject(env, agent.ref_).value(), source);
    EXPECT_TRUE(isOccupied(env, source));

    const auto object = getObjectAtLocation(env, source);
    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(object->type_, Type::xyagent);
    EXPECT_EQ(object->id_, agent.ref_.id_);
}

TEST_F(XYEnvironmentAgentTest, AddRejectsOccupiedLocationWithoutChangingState) {
    XYAgent first;
    XYAgent second;
    ASSERT_TRUE(addObjectToLocation(first, env, source));

    EXPECT_FALSE(addObjectToLocation(second, env, source));

    EXPECT_TRUE(agentIsRegistered(env.agents_, first));
    EXPECT_FALSE(agentIsRegistered(env.agents_, second));
    ASSERT_TRUE(getLocationForObject(env, first.ref_).has_value());
    EXPECT_EQ(getLocationForObject(env, first.ref_).value(), source);
    EXPECT_FALSE(getLocationForObject(env, second.ref_).has_value());
    EXPECT_FALSE(isOccupied(env, destination));
    const auto object = getObjectAtLocation(env, source);
    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(object->id_, first.ref_.id_);
}

TEST_F(XYEnvironmentAgentTest, AddRejectsAlreadyRegisteredAgentAtAnotherLocation) {
    XYAgent agent;
    ASSERT_TRUE(addObjectToLocation(agent, env, source));

    EXPECT_FALSE(addObjectToLocation(agent, env, destination));

    ASSERT_TRUE(getLocationForObject(env, agent.ref_).has_value());
    EXPECT_EQ(getLocationForObject(env, agent.ref_).value(), source);
    EXPECT_TRUE(isOccupied(env, source));
    EXPECT_FALSE(isOccupied(env, destination));
}

TEST_F(XYEnvironmentAgentTest, AddRejectsOutOfBoundsLocationWithoutRegisteringAgent) {
    XYAgent agent;

    EXPECT_FALSE(addObjectToLocation(agent, env, XYLocation(5, 1)));

    EXPECT_FALSE(agentIsRegistered(env.agents_, agent));
    EXPECT_FALSE(getLocationForObject(env, agent.ref_).has_value());
}

TEST_F(XYEnvironmentAgentTest, AddAcceptsBoundaryLocations) {
    XYAgent first;
    XYAgent last;

    EXPECT_TRUE(addObjectToLocation(first, env, XYLocation(1, 1)));
    EXPECT_TRUE(addObjectToLocation(last, env, XYLocation(4, 4)));
}

TEST_F(XYEnvironmentAgentTest, MoveToEmptyLocationVacatesSource) {
    XYAgent agent;
    ASSERT_TRUE(addObjectToLocation(agent, env, source));

    ASSERT_TRUE(moveObjectToLocation(agent, env, destination));

    EXPECT_FALSE(isOccupied(env, source));
    EXPECT_TRUE(isOccupied(env, destination));
    ASSERT_TRUE(getLocationForObject(env, agent.ref_).has_value());
    EXPECT_EQ(getLocationForObject(env, agent.ref_).value(), destination);
    const auto object = getObjectAtLocation(env, destination);
    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(object->id_, agent.ref_.id_);
}

TEST_F(XYEnvironmentAgentTest, MoveRejectsOccupiedDestinationWithoutChangingState) {
    XYAgent moving;
    XYAgent stationary;
    ASSERT_TRUE(addObjectToLocation(moving, env, source));
    ASSERT_TRUE(addObjectToLocation(stationary, env, destination));

    EXPECT_FALSE(moveObjectToLocation(moving, env, destination));

    EXPECT_TRUE(isOccupied(env, source));
    EXPECT_TRUE(isOccupied(env, destination));
    ASSERT_TRUE(getLocationForObject(env, moving.ref_).has_value());
    EXPECT_EQ(getLocationForObject(env, moving.ref_).value(), source);
    const auto sourceObject = getObjectAtLocation(env, source);
    const auto destinationObject = getObjectAtLocation(env, destination);
    ASSERT_TRUE(sourceObject.has_value());
    ASSERT_TRUE(destinationObject.has_value());
    EXPECT_EQ(sourceObject->id_, moving.ref_.id_);
    EXPECT_EQ(destinationObject->id_, stationary.ref_.id_);
}

TEST_F(XYEnvironmentAgentTest, MoveRejectsOutOfBoundsDestinationWithoutChangingState) {
    XYAgent agent;
    ASSERT_TRUE(addObjectToLocation(agent, env, source));

    EXPECT_FALSE(moveObjectToLocation(agent, env, XYLocation(5, 1)));

    EXPECT_TRUE(isOccupied(env, source));
    ASSERT_TRUE(getLocationForObject(env, agent.ref_).has_value());
    EXPECT_EQ(getLocationForObject(env, agent.ref_).value(), source);
}

TEST_F(XYEnvironmentAgentTest, MoveRejectsUnregisteredAgent) {
    XYAgent agent;

    EXPECT_FALSE(moveObjectToLocation(agent, env, destination));

    EXPECT_FALSE(agentIsRegistered(env.agents_, agent));
    EXPECT_FALSE(getLocationForObject(env, agent.ref_).has_value());
    EXPECT_FALSE(isOccupied(env, destination));
}

TEST_F(XYEnvironmentAgentTest, MoveToCurrentLocationFailsWithoutChangingState) {
    XYAgent agent;
    ASSERT_TRUE(addObjectToLocation(agent, env, source));

    EXPECT_FALSE(moveObjectToLocation(agent, env, source));

    EXPECT_TRUE(isOccupied(env, source));
    EXPECT_FALSE(isOccupied(env, destination));
    ASSERT_TRUE(getLocationForObject(env, agent.ref_).has_value());
    EXPECT_EQ(getLocationForObject(env, agent.ref_).value(), source);
}
