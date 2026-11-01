module;

#include <cstddef>
#include <stdexcept>
#include <map>
#include <optional>
#include <iostream>
#include <vector>

export module aima.environment;

namespace environment {
    export struct XYLocation; 
    export struct XYEnvironment;
    export struct XYAgent;
    export struct Wall;
    export struct Dirt;
    export struct Gold;
    export struct TypeRef;
    export enum class Type;
    export enum class Direction;

    using Tiles = std::vector<TypeRef>;
    using ObjRef = std::optional<TypeRef>;
    using Objects = std::map<int, TypeRef>;
    using Locations = std::map<int, XYLocation>;
    using XYSpace = std::map<XYLocation, Tiles>;

    export bool registerObject(Objects& objects, const TypeRef& ref);
    export XYEnvironment makeXYEnvironment(int w, int h);
    export size_t mapSize(const XYEnvironment& env); 
    export bool isOccupied(const XYEnvironment& env, const XYLocation& loc); 
    export bool moveObjectTo(const ObjRef& ref, XYEnvironment& env, const XYLocation& newLocation); 
    export bool moveObject(const ObjRef& ref, XYEnvironment& env, Direction dir); 
    export bool addObjectTo(const TypeRef& ref, XYEnvironment& env, const XYLocation& loc); 
    export std::optional<XYLocation> getLocationFor(const ObjRef& t, const XYEnvironment& env); 
    export bool objectIsRegistered(const ObjRef& t, const Objects& objects); 
    export ObjRef getObjectAt(const XYEnvironment& env, const XYLocation& loc);
    export ObjRef addAgent(XYEnvironment& env, const XYLocation& loc);
    export ObjRef addWall(XYEnvironment& env, const XYLocation& loc);
    export ObjRef addDirt(XYEnvironment& env, const XYLocation& loc);
    export ObjRef addGold(XYEnvironment& env, const XYLocation& loc);

    ///////////////////////////////////////////////////////////////////////////////////////////////

    enum class Type { wall, xyagent, dirt, gold };
    enum class Direction { north, south, east, west };

    struct TypeRef { 
        Type type_; 
        int id; 

        explicit TypeRef(Type t) : type_(t), id(nextId()) {}
        TypeRef() = delete;

    private:
        static int nextId() {
            static int counter = 0;
            return ++counter;
        }
    };

    struct Gold{ 
      TypeRef ref;
      Gold() : ref(Type::gold) {}
    };

    struct Dirt{ 
      TypeRef ref;
      Dirt() : ref(Type::dirt) {}
    };

    struct Wall{ 
      TypeRef ref;
      Wall() : ref(Type::wall) {}
    };

    struct XYAgent{ 
      TypeRef ref;
      XYAgent() : ref(Type::xyagent) {}
    };

    struct XYLocation {
        int x = 0;
        int y = 0;
        XYLocation() = default;
        XYLocation(int x, int y) : x(x), y(y) {}
        auto operator<=>(const XYLocation&) const = default;
        bool operator==(const XYLocation&) const = default;
    };

    struct XYEnvironment {
    private:
        struct FactoryTag {};
        explicit XYEnvironment(FactoryTag) {};
        friend XYEnvironment makeXYEnvironment(int w, int h);

    public:
        XYEnvironment() = delete;
        unsigned w_  = 0;
        unsigned h_ = 0;
        
        Objects objects;
        Locations locations;
        XYSpace xyspace;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////

    export std::ostream& operator<<(std::ostream& os, const TypeRef& t) {
        const char* name = (t.type_ == Type::xyagent) ? "xyagent" : "wall";
        return os << "[" << name << ", " << t.id << "]";
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    export std::ostream& operator<<(std::ostream& os, const ObjRef& t) {
        if (!t) return os << "[none]";
        return os << *t;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    export bool inBounds(const XYEnvironment& env, const XYLocation& loc) {
        return loc.x >= 1 && loc.y >= 1 &&
               loc.x <= static_cast<int>(env.w_) &&
               loc.y <= static_cast<int>(env.h_);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    bool registerObject(Objects& objects, const TypeRef& ref)  {
        return objects.emplace(ref.id, ref).second;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    bool objectIsRegistered(const ObjRef& t, const Objects& objects) {
        if (!t) return false;
        return objects.contains(t->id);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    std::optional<XYLocation> getLocationFor(const ObjRef& t, const XYEnvironment& env) {
        if (!t) return std::nullopt;
        auto it = env.locations.find(t->id);
        if (it == env.locations.end()) return std::nullopt;
        return it->second;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
     
    ObjRef getObjectAt(const XYEnvironment& env, const XYLocation& loc) {
        if (!inBounds(env, loc)) return std::nullopt;

        auto it = env.xyspace.find(loc);
        if (it == env.xyspace.end()) return std::nullopt;

        const Tiles& tiles = it->second;
        if (tiles.empty()) return std::nullopt;

        return tiles.front();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    XYEnvironment makeXYEnvironment(int w, int h) {
        if (w <= 0 || h <= 0) throw std::invalid_argument("params must be > 0");

        XYEnvironment env{XYEnvironment::FactoryTag{}};
        env.w_ = static_cast<unsigned>(w);
        env.h_ = static_cast<unsigned>(h);

        for (int x = 1; x <= w; ++x) {
            for (int y = 1; y <= h; ++y) {
                env.xyspace.emplace(XYLocation(x,y), Tiles()); 
            }
        }

        return env;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    size_t mapSize(const XYEnvironment& env) {
        return env.xyspace.size();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    bool isBlocked(const XYEnvironment& env, const XYLocation& loc) {
        auto it = env.xyspace.find(loc);
        std::vector<TypeRef> refs = it->second;
        
        const auto& ref = refs.front();
        if (ref.type_ == Type::xyagent || ref.type_ == Type::wall) return true;

        return false;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    bool isOccupied(const XYEnvironment& env, const XYLocation& loc) {
        auto it = env.xyspace.find(loc);
        return it != env.xyspace.end() && !it->second.empty(); 
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    bool moveObjectTo(const ObjRef& ref, XYEnvironment& env, const XYLocation& newLocation) {
        auto refIdItr = env.locations.find(ref->id);
        if (refIdItr == env.locations.end()) return false;

        auto newLocItr = env.xyspace.find(newLocation);
        if (newLocItr == env.xyspace.end()) return false;

        if (isOccupied(env, (newLocItr->first))) { 
            if (isBlocked(env, (newLocItr->first))) return false;          
        }
        
        auto oldLocation = (getLocationFor(ref, env));
        if (!oldLocation) return false;
        auto oldLocItr = env.xyspace.find(*oldLocation);

        if (oldLocItr == env.xyspace.end()) return false;
        
        Tiles& tiles = oldLocItr->second;
        std::erase_if(tiles, [&](const TypeRef& r) { return r.id == ref->id; }); // how will I guard against agent dupes

        newLocItr->second.push_back(*ref);
        refIdItr->second = newLocation;
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    bool moveObject(const ObjRef& ref, XYEnvironment& env, Direction dir) {
        XYLocation newLoc(0, 0); 
        XYLocation oldLoc = *(getLocationFor(ref, env));

        try {
            switch (dir) {
                case Direction::north:
                     newLoc.x = oldLoc.x;
                     newLoc.y = oldLoc.y + 1;
                     break;
                case Direction::south:
                     newLoc.x = oldLoc.x;
                     newLoc.y = oldLoc.y - 1;
                     break;
                case Direction::east:
                     newLoc.x = oldLoc.x + 1;
                     newLoc.y = oldLoc.y;
                     break;
                case Direction::west:
                     newLoc.x = oldLoc.x - 1;
                     newLoc.y = oldLoc.y;
                     break;
                default: throw std::runtime_error("Unknown direction");
                         return false;

            }        
        }
        catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << "\n";
        }

        if (moveObjectTo(ref, env, newLoc)) return true;
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    bool addObjectTo(const TypeRef& ref, XYEnvironment& env, const XYLocation& loc) {
        auto it = env.xyspace.find(loc);
        if (it == env.xyspace.end()) return false;

        if (isOccupied(env, loc)) { 
            if (isBlocked(env, loc)) {
                //std::cout << "BLOCKED" << "\n";
                return false;          
            }
        }

        if (!registerObject(env.objects, ref)) {
            //std::cout << "UNREGISTERED" << "\n";
            return false;
        }

        it->second.push_back(ref);
        env.locations[ref.id] = loc;
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    ObjRef addAgent(XYEnvironment& env, const XYLocation& loc) {
        XYAgent agent;
        if (!addObjectTo(agent.ref, env, loc)) return std::nullopt;
        return agent.ref;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    ObjRef addWall(XYEnvironment& env, const XYLocation& loc) {
        Wall wall;
        if (!addObjectTo(wall.ref, env, loc)) return std::nullopt;
        return wall.ref;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    ObjRef addDirt(XYEnvironment& env, const XYLocation& loc) {
        Dirt dirt;
        if (!addObjectTo(dirt.ref, env, loc)) return std::nullopt;
        return dirt.ref;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    ObjRef addGold(XYEnvironment& env, const XYLocation& loc) {
        Gold gold;
        if (!addObjectTo(gold.ref, env, loc)) return std::nullopt;
        return gold.ref;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////


} // namespace environment 























