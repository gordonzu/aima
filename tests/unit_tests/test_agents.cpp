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

    ASSERT_EQ(mapSize(env), size_t(120));
    ASSERT_TRUE(inBounds(env, XYLocation(1, 1)));
    ASSERT_TRUE(inBounds(env, XYLocation(10, 12)));
    ASSERT_FALSE(inBounds(env, XYLocation(11, 5)));
}

TEST_F(XYEnvironmentTest, RejectsNonPositiveDimensions) {
    ASSERT_THROW(makeXYEnvironment(0, 4), std::invalid_argument);
    ASSERT_THROW(makeXYEnvironment(4, 0), std::invalid_argument);
    ASSERT_THROW(makeXYEnvironment(-1, 4), std::invalid_argument);
    ASSERT_THROW(makeXYEnvironment(4, -1), std::invalid_argument);
}

TEST_F(XYEnvironmentTest, BoundsAreOneBasedAndInclusive) {
    ASSERT_TRUE(inBounds(env, XYLocation(1, 1)));
    ASSERT_TRUE(inBounds(env, XYLocation(4, 4)));
    ASSERT_FALSE(inBounds(env, XYLocation(0, 1)));
    ASSERT_FALSE(inBounds(env, XYLocation(1, 0)));
    ASSERT_FALSE(inBounds(env, XYLocation(-1, 1)));
    ASSERT_FALSE(inBounds(env, XYLocation(1, -1)));
    ASSERT_FALSE(inBounds(env, XYLocation(11, 1)));
    ASSERT_FALSE(inBounds(env, XYLocation(1, 14)));
}

TEST_F(XYEnvironmentTest, EmptyAndOccupiedQueriesAgree) {
    const XYLocation loc{2, 2};
    
    ASSERT_FALSE(getObjectAt(env, loc).has_value());
    ASSERT_FALSE(isOccupied(env, loc));

    const auto agent = addAgent(env, loc);

    ASSERT_TRUE(agent.has_value());
    ASSERT_EQ(agent->type_, Type::xyagent);

    ASSERT_TRUE(getObjectAt(env, loc).has_value());
    ASSERT_TRUE(isOccupied(env, loc));
}

TEST_F(XYEnvironmentTest, OutOfBoundsQueriesReturnNoObjectAndAreNotOccupied) {
    for (const XYLocation loc : {XYLocation(0, 1), XYLocation(1, 0),
                                 XYLocation(11, 1), XYLocation(1, 13)}) {
        ASSERT_FALSE(getObjectAt(env, loc).has_value());
        ASSERT_FALSE(isOccupied(env, loc));
    }
}

TEST_F(XYEnvironmentAgentTest, AddRegistersAgentAndRecordsLocation) {
    const auto agent = addAgent(env, source);
    ASSERT_TRUE(agent.has_value());
    ASSERT_EQ(agent->type_, Type::xyagent);

    ASSERT_TRUE(objectIsRegistered(agent, env.objects));
    ASSERT_TRUE(getLocationFor(agent, env).has_value());
    ASSERT_EQ(getLocationFor(agent, env).value(), source);
    ASSERT_TRUE(isOccupied(env, source));

    const auto object = getObjectAt(env, source);
    ASSERT_TRUE(object.has_value());
    ASSERT_EQ(object->type_, Type::xyagent);
    ASSERT_EQ(object->id, agent->id);
}

// I want to return meaningful error messages, not just true/false

TEST_F(XYEnvironmentAgentTest, AddRejectsOccupiedLocationWithoutChangingState) {
    const auto agent1 = addAgent(env, source);
    ASSERT_TRUE(agent1.has_value());
    ASSERT_EQ(agent1->type_, Type::xyagent);

    const auto agent2 = addAgent(env, source);
    ASSERT_FALSE(agent2.has_value());
    ASSERT_THROW(agent2.value(), std::bad_optional_access);

    ASSERT_TRUE(objectIsRegistered(agent1, env.objects));
    ASSERT_FALSE(objectIsRegistered(agent2, env.objects));

    ASSERT_TRUE(getLocationFor(agent1, env).has_value());
    ASSERT_EQ(getLocationFor(agent1, env).value(), source);
    ASSERT_FALSE(getLocationFor(agent2, env).has_value());
    
    const auto object = getObjectAt(env, source);
    ASSERT_TRUE(object.has_value());
    ASSERT_EQ(object->id, agent1->id);
}

TEST_F(XYEnvironmentAgentTest, AddRejectsOutOfBoundsLocationWithoutRegisteringAgent) {
    const auto agent = addAgent(env, XYLocation(11, 12));
    ASSERT_FALSE(agent.has_value());
    ASSERT_THROW(agent.value(), std::bad_optional_access);

    ASSERT_FALSE(objectIsRegistered(agent, env.objects));
    ASSERT_FALSE(getLocationFor(agent, env).has_value());
}

TEST_F(XYEnvironmentAgentTest, AddAcceptsBoundaryLocations) {
    const auto agent1 = addAgent(env, XYLocation(1, 1));
    const auto agent2 = addAgent(env, XYLocation(10, 12));

    ASSERT_TRUE(agent1.has_value()); // write helper function agentState: checks agent has value, is an agent, is registered
    ASSERT_TRUE(agent2.has_value());

    ASSERT_EQ(agent1->type_, Type::xyagent);
    ASSERT_EQ(agent2->type_, Type::xyagent);

    ASSERT_TRUE(objectIsRegistered(agent1, env.objects));
    ASSERT_TRUE(objectIsRegistered(agent2, env.objects));

    ASSERT_EQ(getLocationFor(agent1, env).value(), XYLocation(1, 1));
    ASSERT_EQ(getLocationFor(agent2, env).value(), XYLocation(10, 12));
}

TEST_F(XYEnvironmentAgentTest, MoveToEmptyLocationVacatesSource) {
    const auto agent = addAgent(env, source);

    ASSERT_TRUE(agent.has_value()); // write helper function agentState: checks agent has value, is an agent, is registered
    ASSERT_EQ(agent->type_, Type::xyagent);
    ASSERT_TRUE(objectIsRegistered(agent, env.objects));

    ASSERT_TRUE(moveObjectTo(agent, env, destination));
    ASSERT_FALSE(isOccupied(env, source));
    ASSERT_TRUE(isOccupied(env, destination));

    ASSERT_TRUE(getLocationFor(agent, env).has_value());
    ASSERT_EQ(getLocationFor(agent, env).value(), destination);

    const auto object = getObjectAt(env, destination);
    ASSERT_TRUE(object.has_value());
    ASSERT_EQ(object->id, agent->id);
} 

TEST_F(XYEnvironmentAgentTest, MoveRejectsOccupiedDestinationWithoutChangingState) {
    const auto agent1 = addAgent(env, source);
    const auto agent2 = addAgent(env, destination);

    ASSERT_TRUE(agent1.has_value()); // write helper function agentState: checks agent has value, is an agent, is registered
    ASSERT_TRUE(agent2.has_value());

    ASSERT_EQ(agent1->type_, Type::xyagent);
    ASSERT_EQ(agent2->type_, Type::xyagent);

    ASSERT_FALSE(moveObjectTo(agent1, env, destination));

    ASSERT_TRUE(isOccupied(env, source));
    ASSERT_TRUE(isOccupied(env, destination));
    ASSERT_TRUE(getLocationFor(agent1, env).has_value());
    ASSERT_EQ(getLocationFor(agent1, env).value(), source);

    const auto sourceObject = getObjectAt(env, source);
    const auto destinationObject = getObjectAt(env, destination);

    ASSERT_TRUE(sourceObject.has_value());
    ASSERT_TRUE(destinationObject.has_value());
    ASSERT_EQ(sourceObject->id, agent1->id);
    ASSERT_EQ(destinationObject->id, agent2->id);
}

TEST_F(XYEnvironmentAgentTest, MoveRejectsOutOfBoundsDestinationWithoutChangingState) {
    const auto agent = addAgent(env, source);
    ASSERT_TRUE(agent.has_value());
    ASSERT_EQ(agent->type_, Type::xyagent);

    ASSERT_FALSE(moveObjectTo(agent, env, XYLocation(11, 1)));

    ASSERT_TRUE(isOccupied(env, source));
    ASSERT_TRUE(getLocationFor(agent, env).has_value());
    ASSERT_EQ(getLocationFor(agent, env).value(), source);
}

TEST_F(XYEnvironmentAgentTest, MoveToCurrentLocationFailsWithoutChangingState) {
    const auto agent = addAgent(env, source);
    ASSERT_TRUE(agent.has_value());
    ASSERT_EQ(agent->type_, Type::xyagent);

    ASSERT_FALSE(moveObjectTo(agent, env, source));

    ASSERT_TRUE(isOccupied(env, source));
    ASSERT_FALSE(isOccupied(env, destination));
    ASSERT_TRUE(getLocationFor(agent, env).has_value());
    ASSERT_EQ(getLocationFor(agent, env).value(), source);
}

TEST_F(XYEnvironmentAgentTest, MoveInDirectionToEmptyLocationSucceeds) {
    const auto agent = addAgent(env, XYLocation(3, 5));
    ASSERT_TRUE(agent.has_value());
    ASSERT_EQ(agent->type_, Type::xyagent);

    ASSERT_TRUE(moveObjectTo(agent, env, XYLocation(4, 6)));

    ASSERT_TRUE(moveObject(agent, env, Direction::north));   
    ASSERT_EQ(XYLocation(4, 7), getLocationFor(agent, env));
    ASSERT_TRUE(moveObject(agent, env, Direction::north));   
    ASSERT_EQ(XYLocation(4, 8), getLocationFor(agent, env));

    ASSERT_TRUE(moveObject(agent, env, Direction::west));   
    ASSERT_EQ(XYLocation(3, 8), getLocationFor(agent, env));
    ASSERT_TRUE(moveObject(agent, env, Direction::west));   
    ASSERT_EQ(XYLocation(2, 8), getLocationFor(agent, env));

    ASSERT_TRUE(moveObject(agent, env, Direction::south));   
    ASSERT_EQ(XYLocation(2, 7), getLocationFor(agent, env));
    ASSERT_TRUE(moveObject(agent, env, Direction::south));   
    ASSERT_EQ(XYLocation(2, 6), getLocationFor(agent, env));

    ASSERT_TRUE(moveObject(agent, env, Direction::east));   
    ASSERT_EQ(XYLocation(3, 6), getLocationFor(agent, env));
    ASSERT_TRUE(moveObject(agent, env, Direction::east));   
    ASSERT_EQ(XYLocation(4, 6), getLocationFor(agent, env));
}

TEST_F(XYEnvironmentAgentTest, MoveInDirectionFailsWhenDestinationContainsAgentOrWall) {
    XYLocation loc1(2, 3);
    const auto agent1 = addAgent(env, source);
    const auto agent2 = addAgent(env, loc1);

    ASSERT_TRUE(agent1.has_value()); // write helper function agentState: checks agent has value, is an agent, is registered
    ASSERT_TRUE(agent2.has_value());

    ASSERT_EQ(agent1->type_, Type::xyagent);
    ASSERT_EQ(agent2->type_, Type::xyagent);

    ASSERT_TRUE(isOccupied(env, source));
    ASSERT_TRUE(isOccupied(env, loc1));

    ASSERT_FALSE(moveObject(agent1, env, Direction::north));
    ASSERT_EQ(source, getLocationFor(agent1, env));       
}

TEST_F(XYEnvironmentAgentTest, AddsFailWhenDestinationIsBlocked) {
    XYLocation loc1(5, 5);
    XYLocation loc2(8, 6);
    XYLocation loc3(4, 4);
    XYLocation loc4(1, 1);

    const auto agent = addAgent(env, loc1);
    const auto wall = addWall(env, loc2);
    const auto dirt = addDirt(env, loc3);
    const auto gold = addGold(env, loc4);

    ASSERT_TRUE(agent.has_value()); // write helper function agentState: checks agent has value, is an agent, is registered
    ASSERT_TRUE(wall.has_value());
    ASSERT_TRUE(dirt.has_value());
    ASSERT_TRUE(gold.has_value());

    ASSERT_EQ(agent->type_, Type::xyagent);
    ASSERT_EQ(wall->type_, Type::wall);
    ASSERT_EQ(dirt->type_, Type::dirt);
    ASSERT_EQ(gold->type_, Type::gold);
  
    const auto agent2 = addAgent(env, loc1);
    ASSERT_FALSE(agent2.has_value());

    const auto agent3 = addAgent(env, loc2);
    ASSERT_FALSE(agent3.has_value());

    const auto agent4 = addAgent(env, loc3);
    ASSERT_TRUE(agent4.has_value());

    const auto agent5 = addAgent(env, loc4);
    ASSERT_TRUE(agent5.has_value());
}

TEST_F(XYEnvironmentAgentTest, MovesFailWhenDestinationIsBlocked) {
    XYLocation loc1(5, 5);
    XYLocation loc2(5, 4);
    XYLocation loc3(4, 4);
    XYLocation loc4(4, 5);
    XYLocation loc5(5, 6);

    const auto agent = addAgent(env, loc1);
    const auto agent2 = addAgent(env, loc2);
    const auto dirt = addDirt(env, loc3);
    const auto gold = addGold(env, loc4);
    const auto wall = addWall(env, loc5);

    ASSERT_TRUE(agent.has_value()); // write helper function agentState: checks agent has value, is an agent, is registered
    ASSERT_TRUE(agent2.has_value()); // write helper function agentState: checks agent has value, is an agent, is registered
    ASSERT_TRUE(dirt.has_value());
    ASSERT_TRUE(gold.has_value());
    ASSERT_TRUE(wall.has_value());

    ASSERT_EQ(agent->type_, Type::xyagent);
    ASSERT_EQ(agent2->type_, Type::xyagent);
    ASSERT_EQ(dirt->type_, Type::dirt);
    ASSERT_EQ(gold->type_, Type::gold);
    ASSERT_EQ(wall->type_, Type::wall);

    ASSERT_FALSE(moveObject(agent2, env, Direction::north));
    ASSERT_FALSE(moveObject(agent, env, Direction::north));
    ASSERT_TRUE(moveObject(agent2, env, Direction::west));
    ASSERT_TRUE(moveObject(agent, env, Direction::west));
}





























