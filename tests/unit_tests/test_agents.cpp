#include <gtest/gtest.h> 
#include <sys/ucontext.h>
#include <optional>

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
    XYLocation source, destination;
};

template <typename Objects>
testing::AssertionResult isValidObject(const std::optional<TypeRef>& ref,
                                       const Objects& objects,
                                       Type expected) {
    if (!ref)
        return testing::AssertionFailure() << "object was not created (nullopt)";

    if (!objectIsRegistered(ref, objects))
        return testing::AssertionFailure() << "object id " << ref->id << " is not registered";

    if (ref->type_ != expected)
        return testing::AssertionFailure()
               << "type mismatch: expected " << static_cast<int>(expected)
               << ", got " << static_cast<int>(ref->type_);

    return testing::AssertionSuccess();
}

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
    
    ASSERT_FALSE(getObjectsAt(env, loc).has_value());
    ASSERT_FALSE(isOccupied(env, loc));

    const auto agent = addAgent(env, loc);

    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
    ASSERT_TRUE(getObjectsAt(env, loc).has_value());
    ASSERT_TRUE(isOccupied(env, loc));
}

TEST_F(XYEnvironmentTest, OutOfBoundsQueriesReturnNoObjectAndAreNotOccupied) {
    for (const XYLocation loc : {XYLocation(0, 1), XYLocation(1, 0),
                                 XYLocation(11, 1), XYLocation(1, 13)}) {
        ASSERT_FALSE(getObjectsAt(env, loc).has_value());
        ASSERT_FALSE(isOccupied(env, loc));
    }
}

TEST_F(XYEnvironmentAgentTest, AddRegistersObjectAndRecordsLocation) {
    const auto agent = addAgent(env, source);
    const auto wall = addWall(env, XYLocation(1, 1));
    const auto dirt = addDirt(env, XYLocation(5, 5));
    const auto gold = addGold(env, XYLocation(6, 6));

    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
    ASSERT_TRUE(isValidObject(wall, env.objects, Type::wall));
    ASSERT_TRUE(isValidObject(dirt, env.objects, Type::dirt));
    ASSERT_TRUE(isValidObject(gold, env.objects, Type::gold));

    ASSERT_TRUE(isOccupied(env, source));
    ASSERT_TRUE(isOccupied(env, XYLocation(1, 1)));
    ASSERT_TRUE(isOccupied(env, XYLocation(5, 5)));
    ASSERT_TRUE(isOccupied(env, XYLocation(6, 6)));

    ASSERT_TRUE(getLocationFor(agent, env).has_value());
    ASSERT_TRUE(getLocationFor(wall, env).has_value());
    ASSERT_TRUE(getLocationFor(dirt, env).has_value());
    ASSERT_TRUE(getLocationFor(gold, env).has_value());

    ASSERT_EQ(getLocationFor(agent, env).value(), source);
    ASSERT_EQ(getLocationFor(wall, env).value(), XYLocation(1, 1));
    ASSERT_EQ(getLocationFor(dirt, env).value(), XYLocation(5, 5));
    ASSERT_EQ(getLocationFor(gold, env).value(), XYLocation(6, 6));

    const auto object1 = getObjectsAt(env, source);
    ASSERT_EQ(object1->id, agent->id);
    const auto object2 = getObjectsAt(env, XYLocation(1, 1));
    ASSERT_EQ(object2->id, wall->id);
    const auto object3 = getObjectsAt(env, XYLocation(5, 5));
    ASSERT_EQ(object3->id, dirt->id);
    const auto object4 = getObjectsAt(env, XYLocation(6, 6));
    ASSERT_EQ(object4->id, gold->id);
}

TEST_F(XYEnvironmentAgentTest, AddPlacesMutltipleObjectsAtLocation) {
    const auto agent = addAgent(env, source);
    const auto agent2 = addAgent(env, source);
    const auto wall = addWall(env, source);
    const auto dirt = addDirt(env, source);
    const auto gold = addGold(env, source);

    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
    ASSERT_TRUE(isValidObject(agent2, env.objects, Type::xyagent));
    ASSERT_TRUE(isValidObject(wall, env.objects, Type::wall));
    ASSERT_TRUE(isValidObject(dirt, env.objects, Type::dirt));
    ASSERT_TRUE(isValidObject(gold, env.objects, Type::gold));


    ASSERT_EQ(getLocationFor(agent, env).value(), source);
    ASSERT_EQ(getLocationFor(agent2, env).value(), source);
    ASSERT_EQ(getLocationFor(wall, env).value(), source);
    ASSERT_EQ(getLocationFor(dirt, env).value(), source);
    ASSERT_EQ(getLocationFor(gold, env).value(), source);
}

TEST_F(XYEnvironmentAgentTest, AddRejectsOutOfBoundsLocationWithoutRegisteringAgent) {
    const auto agent = addAgent(env, XYLocation(11, 12));
    ASSERT_FALSE(isValidObject(agent, env.objects, Type::xyagent));
    ASSERT_THROW(agent.value(), std::bad_optional_access);

    ASSERT_FALSE(objectIsRegistered(agent, env.objects));
    ASSERT_FALSE(getLocationFor(agent, env).has_value());
}

TEST_F(XYEnvironmentAgentTest, AddAcceptsBoundaryLocations) {
    const auto agent = addAgent(env, XYLocation(1, 1));
    const auto wall = addWall(env, XYLocation(10, 12));

    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
    ASSERT_TRUE(isValidObject(wall, env.objects, Type::wall));
    
    ASSERT_EQ(getLocationFor(agent, env).value(), XYLocation(1, 1));
    ASSERT_EQ(getLocationFor(wall, env).value(), XYLocation(10, 12));
}

TEST_F(XYEnvironmentAgentTest, MoveToEmptyLocationSucceedsAndVacatesSource) {
    const auto agent = addAgent(env, source);
    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));

    ASSERT_TRUE(moveObjectTo(agent, env, destination));
    ASSERT_FALSE(isOccupied(env, source));
    ASSERT_TRUE(isOccupied(env, destination));

    ASSERT_TRUE(getLocationFor(agent, env).has_value());
    ASSERT_EQ(getLocationFor(agent, env).value(), destination);

    const auto object = getObjectsAt(env, destination);
    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
    ASSERT_EQ(object->id, agent->id);
} 

TEST_F(XYEnvironmentAgentTest, MoveToOccupiedLocationSucceedsAndVacatesSource) {
    const auto agent = addAgent(env, XYLocation(5, 6));
    const auto wall = addWall(env, XYLocation(2, 1));
    const auto dirt = addDirt(env, XYLocation(2, 1));
    const auto gold = addGold(env, XYLocation(2, 1));

    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
    ASSERT_TRUE(isValidObject(wall, env.objects, Type::wall));
    ASSERT_TRUE(isValidObject(dirt, env.objects, Type::dirt));
    ASSERT_TRUE(isValidObject(gold, env.objects, Type::gold));

    ASSERT_TRUE(moveObjectTo(agent, env, XYLocation(2, 1)));
    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
    ASSERT_TRUE(getLocationFor(agent, env).has_value());
    ASSERT_EQ(getLocationFor(agent, env).value(), XYLocation(2, 1));

    ASSERT_FALSE(getObjectsAt(env, XYLocation(5, 6))); 
}

TEST_F(XYEnvironmentAgentTest, MoveToOutOfBoundsDestinationFailsWithoutChangingState) {
    const auto agent = addAgent(env, source);
    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));

    ASSERT_FALSE(moveObjectTo(agent, env, XYLocation(11, 1)));

    ASSERT_TRUE(isOccupied(env, source));
    ASSERT_TRUE(getLocationFor(agent, env).has_value());
    ASSERT_EQ(getLocationFor(agent, env).value(), source);
}

TEST_F(XYEnvironmentAgentTest, MoveInDirectionToEmptyOrNonWallLocationSucceeds) {
    const auto agent = addAgent(env, XYLocation(3, 5));

    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
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

    const auto wall = addWall(env, XYLocation(4, 7));
    ASSERT_TRUE(isValidObject(wall, env.objects, Type::wall));
    ASSERT_FALSE(moveObject(agent, env, Direction::north));

    const auto agent2 = addAgent(env, XYLocation(4, 5));
    ASSERT_TRUE(isValidObject(agent2, env.objects, Type::xyagent));
    ASSERT_TRUE(moveObject(agent2, env, Direction::north));   

    const auto dirt = addDirt(env, XYLocation(4, 5));
    const auto gold = addGold(env, XYLocation(4, 5));

    ASSERT_TRUE(isValidObject(dirt, env.objects, Type::dirt));
    ASSERT_TRUE(isValidObject(gold, env.objects, Type::gold));
    ASSERT_TRUE(moveObject(agent2, env, Direction::south));   
}

TEST_F(XYEnvironmentAgentTest, MoveInDirectionToLocationBlockedByWallFails) {
    XYLocation loc1(2, 3);
    XYLocation loc2(2, 1);
    XYLocation loc3(3, 2);
    XYLocation loc4(1, 2);

    const auto agent = addAgent(env, source);
    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));

    const auto wall1 = addWall(env, loc1);
    ASSERT_TRUE(isValidObject(wall1, env.objects, Type::wall));
    const auto wall2 = addWall(env, loc2);
    ASSERT_TRUE(isValidObject(wall2, env.objects, Type::wall));
    const auto wall3 = addWall(env, loc3);
    ASSERT_TRUE(isValidObject(wall3, env.objects, Type::wall));
    const auto wall4 = addWall(env, loc4);
    ASSERT_TRUE(isValidObject(wall4, env.objects, Type::wall));

    ASSERT_FALSE(moveObject(agent, env, Direction::north));
    ASSERT_FALSE(moveObject(agent, env, Direction::south));
    ASSERT_FALSE(moveObject(agent, env, Direction::east));
    ASSERT_FALSE(moveObject(agent, env, Direction::west));

    ASSERT_EQ(source, getLocationFor(agent, env));       
}

// figure out TypeRef equality, equivalence 


/*TEST_F(XYEnvironmentAgentTest, MovesFailWhenDestinationIsBlocked) {
    XYLocation loc1(5, 5);
    XYLocation loc2(5, 4);
    XYLocation loc3(4, 4);
    XYLocation loc4(4, 5);
    XYLocation loc5(5, 6);

    const auto agent = addAgent(env, loc1);
    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
    const auto agent2 = addAgent(env, loc2);
    ASSERT_TRUE(isValidObject(agent2, env.objects, Type::xyagent));
    const auto dirt = addDirt(env, loc3);
    ASSERT_TRUE(isValidObject(dirt, env.objects, Type::dirt));
    const auto gold = addGold(env, loc4);
    ASSERT_TRUE(isValidObject(gold, env.objects, Type::gold));
    const auto wall = addWall(env, loc5);
    ASSERT_TRUE(isValidObject(wall, env.objects, Type::wall));

    ASSERT_FALSE(moveObject(agent2, env, Direction::north));
    ASSERT_FALSE(moveObject(agent, env, Direction::north));
    ASSERT_TRUE(moveObject(agent2, env, Direction::west));
    ASSERT_TRUE(moveObject(agent, env, Direction::west));
}*/

/*TEST_F(XYEnvironmentAgentTest, AddRejectsOccupiedLocationWithoutChangingState) {
    const auto agent1 = addAgent(env, source);
    ASSERT_TRUE(isValidObject(agent1, env.objects, Type::xyagent));

    const auto agent2 = addAgent(env, source);
    ASSERT_FALSE(isValidObject(agent2, env.objects, Type::xyagent));
    ASSERT_THROW(agent2.value(), std::bad_optional_access);

    ASSERT_TRUE(getLocationFor(agent1, env).has_value());
    ASSERT_FALSE(getLocationFor(agent2, env).has_value());
    
    const auto object1 = getObjectAt(env, source);
    ASSERT_TRUE(isValidObject(object1, env.objects, Type::xyagent));
    ASSERT_EQ(object1->id, agent1->id);
}*/

/*TEST_F(XYEnvironmentAgentTest, MoveRejectsOccupiedDestinationWithoutChangingState) {
    const auto agent = addAgent(env, source);
    const auto wall = addWall(env, destination);
    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
    ASSERT_TRUE(isValidObject(wall, env.objects, Type::wall));

    ASSERT_FALSE(moveObjectTo(agent, env, destination));

    ASSERT_TRUE(isOccupied(env, source));
    ASSERT_TRUE(isOccupied(env, destination));
    ASSERT_TRUE(getLocationFor(agent, env).has_value());
    ASSERT_EQ(getLocationFor(agent, env).value(), source);

    const auto sourceObject = getObjectAt(env, source);
    const auto destinationObject = getObjectAt(env, destination);

    ASSERT_TRUE(sourceObject.has_value());
    ASSERT_TRUE(destinationObject.has_value());
    ASSERT_EQ(sourceObject->id, agent->id);
    ASSERT_EQ(destinationObject->id, wall->id);
}*/

/*TEST_F(XYEnvironmentAgentTest, MoveToBlockedLocationFailsWithoutChangingState) {
    const auto agent = addAgent(env, source);
    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
    const auto wall = addWall(env, destination);
    ASSERT_TRUE(isValidObject(wall, env.objects, Type::wall));

    ASSERT_FALSE(moveObjectTo(agent, env, source));

    ASSERT_TRUE(isOccupied(env, source));
    ASSERT_TRUE(getLocationFor(agent, env).has_value());
    ASSERT_EQ(getLocationFor(agent, env).value(), source);
}*/

/*TEST_F(XYEnvironmentAgentTest, AddsToBlockedLocationsFail) {
    XYLocation loc1(5, 5);
    XYLocation loc2(8, 6);
    XYLocation loc3(4, 4);
    XYLocation loc4(1, 1);

    const auto agent = addAgent(env, loc1);
    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
    const auto wall = addWall(env, loc2);
    ASSERT_TRUE(isValidObject(wall, env.objects, Type::wall));
    const auto dirt = addDirt(env, loc3);
    ASSERT_TRUE(isValidObject(dirt, env.objects, Type::dirt));
    const auto gold = addGold(env, loc4);
    ASSERT_TRUE(isValidObject(gold, env.objects, Type::gold));

    const auto agent2 = addAgent(env, loc1);
    ASSERT_FALSE(agent2.has_value());

    const auto agent3 = addAgent(env, loc2);
    ASSERT_FALSE(agent3.has_value());

    const auto agent4 = addAgent(env, loc3);
    ASSERT_TRUE(agent4.has_value());

    const auto agent5 = addAgent(env, loc4);
    ASSERT_TRUE(agent5.has_value());
}*/










