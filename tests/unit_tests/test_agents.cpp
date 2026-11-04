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
    
    ASSERT_TRUE(getSetOfObjectsAt(env, loc).empty());
    ASSERT_FALSE(isOccupied(env, loc));

    const auto agent = addAgent(env, loc);

    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
    ASSERT_EQ(getSetOfObjectsAt(env, loc).size(), 1);
    ASSERT_TRUE(isOccupied(env, loc));
}

TEST_F(XYEnvironmentTest, OutOfBoundsQueriesThrowAndAreNotOccupied) {
    for (const XYLocation loc : {XYLocation(11, 13), XYLocation(25, 25),
                                 XYLocation(11, 1), XYLocation(1, 13)}) {
        ASSERT_THROW(getSetOfObjectsAt(env, loc), std::invalid_argument);
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

    
    const auto objects = getSetOfObjectsAt(env, source);
    ASSERT_TRUE(objects.size() == 1);
    ASSERT_TRUE(objects.contains(*agent));

    const auto objects2 = getSetOfObjectsAt(env, XYLocation(1, 1));
    ASSERT_TRUE(objects2.size() == 1);
    ASSERT_TRUE(objects2.contains(*wall));

    const auto objects3 = getSetOfObjectsAt(env, XYLocation(5, 5));
    ASSERT_TRUE(objects3.size() == 1);
    ASSERT_TRUE(objects3.contains(*dirt));

    const auto objects4 = getSetOfObjectsAt(env, XYLocation(6, 6));
    ASSERT_TRUE(objects4.size() == 1);
    ASSERT_TRUE(objects4.contains(*gold));
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

    const auto objects = getSetOfObjectsAt(env, destination);
    const auto it = objects.find(*agent);
    ASSERT_TRUE(it != objects.end());
    ASSERT_EQ(it->id, agent->id);
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

    ASSERT_TRUE(getSetOfObjectsAt(env, XYLocation(5, 6)).size() == 0);
}

TEST_F(XYEnvironmentAgentTest, GetSetOfObjectsAtReturnsAllObjectsAtLocation) {
    const auto agent = addAgent(env, XYLocation(6, 6));
    const auto wall = addWall(env, XYLocation(6, 6));
    const auto dirt = addDirt(env, XYLocation(6, 6));
    const auto gold = addGold(env, XYLocation(6, 6));

    ASSERT_EQ(getSetOfObjectsAt(env, XYLocation(6, 6)).size(), 4);
}

TEST_F(XYEnvironmentAgentTest, GetSetOfObjectsAtReturnsEmptySetAtEmptyThrowsAtOutOfBoundsLocations) {
    ASSERT_EQ(getSetOfObjectsAt(env, XYLocation(10, 10)).size(), 0);
    ASSERT_THROW(getSetOfObjectsAt(env, XYLocation(15, 20)), std::invalid_argument);
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

TEST_F(XYEnvironmentAgentTest, GetObjectsNearReturnsAllNeighboringObjectsInRadius) {
    const auto agent = addAgent(env, XYLocation(5, 5));
    ASSERT_TRUE(isValidObject(agent, env.objects, Type::xyagent));
    const auto agent2 = addAgent(env, XYLocation(7, 4));
    ASSERT_TRUE(isValidObject(agent2, env.objects, Type::xyagent));
    const auto agent3 = addAgent(env, XYLocation(5, 7));
    ASSERT_TRUE(isValidObject(agent3, env.objects, Type::xyagent));
    const auto wall = addWall(env, XYLocation(3, 10));
    ASSERT_TRUE(isValidObject(wall, env.objects, Type::wall));

    std::set<TypeRef> visibleToAgent = getObjectsNear(agent, env, 3);
    ASSERT_TRUE(visibleToAgent.size() == 2);

    std::set<TypeRef> visibleToAgent2 = getObjectsNear(agent2, env, 3);
    ASSERT_TRUE(visibleToAgent2.size() == 1);

    ASSERT_TRUE(moveObject(agent2, env, Direction::north));
    visibleToAgent2 = getObjectsNear(agent2, env, 3);
    ASSERT_TRUE(visibleToAgent2.size() == 2);

    ASSERT_TRUE(moveObjectTo(agent3, env, XYLocation(3, 11)));
    std::set<TypeRef> visibleToAgent3 = getObjectsNear(agent3, env, 3);
    ASSERT_TRUE(visibleToAgent3.size() == 1);
}

TEST_F(XYEnvironmentAgentTest, CreateWalledPerimeter) {
   ASSERT_TRUE(makePerimeter(env));

   ASSERT_TRUE(isBlocked(env, XYLocation(1, 1)));
   ASSERT_TRUE(isBlocked(env, XYLocation(1, 7)));
   ASSERT_TRUE(isBlocked(env, XYLocation(1, 12)));
   ASSERT_TRUE(isBlocked(env, XYLocation(7, 1)));
   ASSERT_TRUE(isBlocked(env, XYLocation(10, 1)));
   ASSERT_TRUE(isBlocked(env, XYLocation(10, 7)));
   ASSERT_TRUE(isBlocked(env, XYLocation(10, 12)));
   ASSERT_TRUE(isBlocked(env, XYLocation(7, 12)));
}
























// figure out TypeRef equality, equivalence 



