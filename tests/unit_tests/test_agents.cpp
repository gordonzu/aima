#include <gtest/gtest.h> 
#include <sys/ucontext.h>

import aima.environment;
import aima.agent;

using namespace environment;

class XYEnvironmentTest : public testing::Test {
protected:
    XYEnvironment env = makeXYEnvironment(10, 12);
};

class XYEnvironmentAgentTest : public testing::Test {
protected:
    XYEnvironmentAgentTest() : env(makeXYEnvironment(10, 12)), 
                               source(2, 2),
                               destination(3, 3) {}

    XYEnvironment env;
    XYLocation source;
    XYLocation destination;
};

TEST_F(XYEnvironmentTest, ConstructXYEnvironmentGrid) {

    EXPECT_EQ(mapSize(env), size_t(120));
    EXPECT_TRUE(inBounds(env, XYLocation(1, 1)));
    EXPECT_TRUE(inBounds(env, XYLocation(10, 12)));
    EXPECT_FALSE(inBounds(env, XYLocation(11, 5)));
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
    EXPECT_FALSE(inBounds(env, XYLocation(11, 1)));
    EXPECT_FALSE(inBounds(env, XYLocation(1, 14)));
}

TEST_F(XYEnvironmentTest, EmptyAndOccupiedQueriesAgree) {
    const XYLocation loc{2, 2};
    
    EXPECT_FALSE(getObjectAt(env, loc).has_value());
    EXPECT_FALSE(isOccupied(env, loc));

    auto r = addAgent(env, loc);
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->type_, Type::xyagent);

    EXPECT_TRUE(getObjectAt(env, loc).has_value());
    EXPECT_TRUE(isOccupied(env, loc));
}

TEST_F(XYEnvironmentTest, OutOfBoundsQueriesReturnNoObjectAndAreNotOccupied) {
    for (const XYLocation loc : {XYLocation(0, 1), XYLocation(1, 0),
                                 XYLocation(11, 1), XYLocation(1, 13)}) {
        EXPECT_FALSE(getObjectAt(env, loc).has_value());
        EXPECT_FALSE(isOccupied(env, loc));
    }
}

/*TEST_F(XYEnvironmentAgentTest, AddRegistersAgentAndRecordsLocation) {
    auto r = addAgent(env, source);

    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->type_, Type::xyagent);

    EXPECT_TRUE(objectIsRegistered(env.objects, r));
    ASSERT_TRUE(getLocationFor(*r, env).has_value());
    EXPECT_EQ(getLocationFor(*r, env).value(), source);
    EXPECT_TRUE(isOccupied(env, source));

    const auto object = getObjectAt(env, source);
    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(object->type_, Type::xyagent);
    EXPECT_EQ(object->id, r->id);
}*/

TEST_F(XYEnvironmentAgentTest, AddRejectsOccupiedLocationWithoutChangingState) {
    auto r1 = addAgent(env, source);
    auto r2 = addAgent(env, source);

    ASSERT_TRUE(r1.has_value());
    EXPECT_FALSE(r2.has_value());

    EXPECT_TRUE(objectIsRegistered(r1, env.objects));
    EXPECT_FALSE(objectIsRegistered(r2, env.objects));

    ASSERT_TRUE(getLocationFor(r1, env).has_value());
    EXPECT_EQ(getLocationFor(r1, env).value(), source);
    /*EXPECT_FALSE(getLocationFor(*r2, env).has_value());
    EXPECT_FALSE(isOccupied(env, destination));
    
    const auto object = getObjectAt(env, source);
    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(object->id, r1->id);
    */
}

/*
TEST_F(XYEnvironmentAgentTest, AddRejectsAlreadyRegisteredAgentAtAnotherLocation) {
    XYAgent agent;

    ASSERT_TRUE(addObjectTo(agent.ref, env, source));
    EXPECT_FALSE(addObjectTo(agent.ref, env, destination));

    ASSERT_TRUE(getLocationFor(agent.ref, env).has_value());
    EXPECT_EQ(getLocationFor(agent.ref, env).value(), source);
    EXPECT_TRUE(isOccupied(env, source));
    EXPECT_FALSE(isOccupied(env, destination));
}

TEST_F(XYEnvironmentAgentTest, AddRejectsOutOfBoundsLocationWithoutRegisteringAgent) {
    XYAgent agent;

    EXPECT_FALSE(addObjectTo(agent.ref, env, XYLocation(11, 1)));
    EXPECT_FALSE(objectIsRegistered(env.objects, agent.ref));
    EXPECT_FALSE(getLocationFor(agent.ref, env).has_value());
}

TEST_F(XYEnvironmentAgentTest, AddAcceptsBoundaryLocations) {
    XYAgent first;
    XYAgent last;

    EXPECT_TRUE(addObjectTo(first.ref, env, XYLocation(1, 1)));
    EXPECT_TRUE(addObjectTo(last.ref, env, XYLocation(4, 4)));
}

TEST_F(XYEnvironmentAgentTest, MoveToEmptyLocationVacatesSource) {
    XYAgent agent;

    ASSERT_TRUE(addObjectTo(agent.ref, env, source));
    ASSERT_TRUE(moveObjectTo(agent.ref, env, destination));

    EXPECT_FALSE(isOccupied(env, source));
    EXPECT_TRUE(isOccupied(env, destination));
    ASSERT_TRUE(getLocationFor(agent.ref, env).has_value());
    EXPECT_EQ(getLocationFor(agent.ref, env).value(), destination);

    const auto object = getObjectAt(env, destination);
    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(object->id, agent.ref.id);
} 

TEST_F(XYEnvironmentAgentTest, MoveRejectsOccupiedDestinationWithoutChangingState) {
    XYAgent moving;
    XYAgent stationary;

    ASSERT_TRUE(addObjectTo(moving.ref, env, source));
    ASSERT_TRUE(addObjectTo(stationary.ref, env, destination));

    EXPECT_FALSE(moveObjectTo(moving.ref, env, destination));

    EXPECT_TRUE(isOccupied(env, source));
    EXPECT_TRUE(isOccupied(env, destination));
    ASSERT_TRUE(getLocationFor(moving.ref, env).has_value());
    EXPECT_EQ(getLocationFor(moving.ref, env).value(), source);

    const auto sourceObject = getObjectAt(env, source);
    const auto destinationObject = getObjectAt(env, destination);

    ASSERT_TRUE(sourceObject.has_value());
    ASSERT_TRUE(destinationObject.has_value());
    EXPECT_EQ(sourceObject->id, moving.ref.id);
    EXPECT_EQ(destinationObject->id, stationary.ref.id);
}

TEST_F(XYEnvironmentAgentTest, MoveRejectsOutOfBoundsDestinationWithoutChangingState) {
    XYAgent agent;

    ASSERT_TRUE(addObjectTo(agent.ref, env, source));
    EXPECT_FALSE(moveObjectTo(agent.ref, env, XYLocation(11, 1)));

    EXPECT_TRUE(isOccupied(env, source));
    ASSERT_TRUE(getLocationFor(agent.ref, env).has_value());
    EXPECT_EQ(getLocationFor(agent.ref, env).value(), source);
}

TEST_F(XYEnvironmentAgentTest, MoveRejectsUnregisteredAgent) {
    XYAgent agent;

    EXPECT_FALSE(moveObjectTo(agent.ref, env, destination));

    EXPECT_FALSE(objectIsRegistered(env.objects, agent.ref));
    EXPECT_FALSE(getLocationFor(agent.ref, env).has_value());
    EXPECT_FALSE(isOccupied(env, destination));
}

TEST_F(XYEnvironmentAgentTest, MoveToCurrentLocationFailsWithoutChangingState) {
    XYAgent agent;

    ASSERT_TRUE(addObjectTo(agent.ref, env, source));
    EXPECT_FALSE(moveObjectTo(agent.ref, env, source));

    EXPECT_TRUE(isOccupied(env, source));
    EXPECT_FALSE(isOccupied(env, destination));
    ASSERT_TRUE(getLocationFor(agent.ref, env).has_value());
    EXPECT_EQ(getLocationFor(agent.ref, env).value(), source);
}

TEST_F(XYEnvironmentAgentTest, MoveInDirectionToEmptyLocationSucceeds) {
    XYAgent agent;
    TypeRef t = agent.ref;

    ASSERT_TRUE(addObjectTo(t, env, XYLocation(3, 5)));
    ASSERT_TRUE(moveObjectTo(t, env, XYLocation(4, 6)));

    ASSERT_TRUE(moveObject(t, env, Direction::north));   
    EXPECT_EQ(XYLocation(4, 7), getLocationFor(t, env));
    ASSERT_TRUE(moveObject(t, env, Direction::north));   
    EXPECT_EQ(XYLocation(4, 8), getLocationFor(t, env));

    ASSERT_TRUE(moveObject(t, env, Direction::west));   
    EXPECT_EQ(XYLocation(3, 8), getLocationFor(t, env));
    ASSERT_TRUE(moveObject(t, env, Direction::west));   
    EXPECT_EQ(XYLocation(2, 8), getLocationFor(t, env));

    ASSERT_TRUE(moveObject(t, env, Direction::south));   
    EXPECT_EQ(XYLocation(2, 7), getLocationFor(t, env));
    ASSERT_TRUE(moveObject(t, env, Direction::south));   
    EXPECT_EQ(XYLocation(2, 6), getLocationFor(t, env));

    ASSERT_TRUE(moveObject(t, env, Direction::east));   
    EXPECT_EQ(XYLocation(3, 6), getLocationFor(t, env));
    ASSERT_TRUE(moveObject(t, env, Direction::east));   
    EXPECT_EQ(XYLocation(4, 6), getLocationFor(t, env));
}

TEST_F(XYEnvironmentAgentTest, MoveInDirectionFailsWhenDestinationContainsAgentOrWall) {
    XYAgent agent1;
    XYAgent agent2;
    XYLocation loc1(2, 3);

    ASSERT_TRUE(addObjectTo(agent1.ref, env, source));
    ASSERT_TRUE(isOccupied(env, source));
    ASSERT_TRUE(addObjectTo(agent2.ref, env, loc1));
    ASSERT_TRUE(isOccupied(env, loc1));

    EXPECT_FALSE(moveObject(agent1.ref, env, Direction::north));
    EXPECT_EQ(source, *(getLocationFor(agent1.ref, env)));       
}

TEST_F(XYEnvironmentAgentTest, AddsFailWhenDestinationIsBlocked) {
    XYAgent agent;
    XYAgent agent2;
    XYAgent agent3;
    Wall wall;
    Dirt dirt;
    Gold gold;

    XYLocation loc1(5, 5);
    XYLocation loc2(8, 6);
    XYLocation loc3(4, 4);
    XYLocation loc4(1, 1);

    ASSERT_TRUE(addObjectTo(agent.ref, env, loc1));
    ASSERT_TRUE(addObjectTo(wall.ref, env, loc2));
    ASSERT_TRUE(addObjectTo(dirt.ref, env, loc3));
    ASSERT_TRUE(addObjectTo(gold.ref, env, loc4));

    EXPECT_FALSE(addObjectTo(agent2.ref, env, loc1));
    EXPECT_FALSE(addObjectTo(agent2.ref, env, loc2));
    ASSERT_TRUE(addObjectTo(agent2.ref, env, loc3));
    ASSERT_TRUE(addObjectTo(agent3.ref, env, loc4));
}

TEST_F(XYEnvironmentAgentTest, MovesFailWhenDestinationIsBlocked) {
    XYAgent agent;
    XYAgent agent2;
    Wall wall;
    Dirt dirt;
    Gold gold;

    XYLocation loc1(5, 5);
    XYLocation loc2(5, 4);
    XYLocation loc3(4, 4);
    XYLocation loc4(1, 1);

    ASSERT_TRUE(addObjectTo(agent.ref, env, loc1));
    ASSERT_TRUE(addObjectTo(agent2.ref, env, loc2));
    ASSERT_TRUE(addObjectTo(dirt.ref, env, loc3));
    ASSERT_TRUE(addObjectTo(gold.ref, env, loc4));

    EXPECT_FALSE(moveObject(agent2.ref, env, Direction::north));
}

*/



























