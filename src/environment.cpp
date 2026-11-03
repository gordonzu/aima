module;

#include <cstddef>
#include <stdexcept>
#include <map>
#include <optional>
#include <iostream>
#include <set>

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

    using ObjectSet = std::set<TypeRef>;
    using ObjectRef = std::optional<TypeRef>;
    using Objects = std::map<int, TypeRef>;
    using Locations = std::map<int, XYLocation>;
    using XYSpace = std::map<XYLocation, ObjectSet>;

    export bool registerObject(Objects& objects, const TypeRef& ref);
    export XYEnvironment makeXYEnvironment(int w, int h);
    export size_t mapSize(const XYEnvironment& env); 
    export bool isOccupied(const XYEnvironment& env, const XYLocation& loc); 
    export bool moveObjectTo(const ObjectRef& ref, XYEnvironment& env, const XYLocation& newLocation); 
    export bool moveObject(const ObjectRef& ref, XYEnvironment& env, Direction dir); 
    bool addObjectTo(const TypeRef& ref, XYEnvironment& env, const XYLocation& loc); 
    export std::optional<XYLocation> getLocationFor(const ObjectRef& t, const XYEnvironment& env); 
    export bool objectIsRegistered(const ObjectRef& t, const Objects& objects); 
    export ObjectSet getSetOfObjectsAt(const XYEnvironment& env, const XYLocation& loc);
    export ObjectRef addAgent(XYEnvironment& env, const XYLocation& loc);
    export ObjectRef addWall(XYEnvironment& env, const XYLocation& loc);
    export ObjectRef addDirt(XYEnvironment& env, const XYLocation& loc);
    export ObjectRef addGold(XYEnvironment& env, const XYLocation& loc);
    template <typename T> ObjectRef addObject(XYEnvironment& env, const XYLocation& loc);

    enum class Type { wall, xyagent, dirt, gold };
    enum class Direction { north, south, east, west };

    struct TypeRef { 
        Type type_; 
        int id; 

        explicit TypeRef(Type t) : type_(t), id(nextId()) {}
        TypeRef() = delete;

        friend bool operator<(const TypeRef& a, const TypeRef& b) { return a.id < b.id; }

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
      bool isAlive = true;
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

    export std::ostream& operator<<(std::ostream& os, const TypeRef& t) {
        const char* name = (t.type_ == Type::xyagent) ? "xyagent" : "wall";
        return os << "[" << name << ", " << t.id << "]";
    }

    export std::ostream& operator<<(std::ostream& os, const ObjectRef& t) {
        if (!t) return os << "[none]";
        return os << *t;
    }

    export bool inBounds(const XYEnvironment& env, const XYLocation& loc) {
        return loc.x >= 1 && loc.y >= 1 &&
               loc.x <= static_cast<int>(env.w_) &&
               loc.y <= static_cast<int>(env.h_);
    }

    bool registerObject(Objects& objects, const TypeRef& ref)  {
        return objects.emplace(ref.id, ref).second;
    }

    bool objectIsRegistered(const ObjectRef& t, const Objects& objects) {
        if (!t) return false;
        return objects.contains(t->id);
    }

    std::optional<XYLocation> getLocationFor(const ObjectRef& t, const XYEnvironment& env) {
        if (!t) return std::nullopt;
        auto it = env.locations.find(t->id);
        if (it == env.locations.end()) return std::nullopt;
        return it->second;
    }

    ObjectSet getSetOfObjectsAt(const XYEnvironment& env, const XYLocation& loc) {
        if (!inBounds(env, loc)) throw std::invalid_argument("XYLocation must be within matrix bounds");
        auto it = env.xyspace.find(loc);
        if (it == env.xyspace.end()) return {};
        return ObjectSet(it->second.begin(), it->second.end());
    }

    XYEnvironment makeXYEnvironment(int w, int h) {
        if (w <= 0 || h <= 0) throw std::invalid_argument("params must be > 0");

        XYEnvironment env{XYEnvironment::FactoryTag{}};
        env.w_ = static_cast<unsigned>(w);
        env.h_ = static_cast<unsigned>(h);

        for (int x = 1; x <= w; ++x) {
            for (int y = 1; y <= h; ++y) {
                env.xyspace.emplace(XYLocation(x,y), ObjectSet()); 
            }
        }

        return env;
    }

    size_t mapSize(const XYEnvironment& env) {
        return env.xyspace.size();
    }

    bool isBlocked(const XYEnvironment& env, const XYLocation& loc) {
        auto it = env.xyspace.find(loc);
        std::set<TypeRef> refs = it->second;
        
        for (const auto r : refs) {
            if (r.type_ == Type::wall) return true;
        }

        return false;
    }

    bool isOccupied(const XYEnvironment& env, const XYLocation& loc) {
        auto it = env.xyspace.find(loc);
        return it != env.xyspace.end() && !it->second.empty(); 
    }

    bool moveObjectTo(const ObjectRef& ref, XYEnvironment& env, const XYLocation& newLocation) {
        auto refIdItr = env.locations.find(ref->id);
        if (refIdItr == env.locations.end()) return false;

        auto newLocItr = env.xyspace.find(newLocation);
        if (newLocItr == env.xyspace.end()) return false;

        auto oldLocation = (getLocationFor(ref, env));
        if (!oldLocation) return false;
        auto oldLocItr = env.xyspace.find(*oldLocation);

        if (oldLocItr == env.xyspace.end()) return false;
        
        ObjectSet& objects = oldLocItr->second;
        objects.erase(*ref);

        newLocItr->second.emplace(*ref);
        refIdItr->second = newLocation;
        return true;
    }

    bool moveObject(const ObjectRef& ref, XYEnvironment& env, Direction dir) {
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

        if (isOccupied(env, newLoc)) { 
            if (isBlocked(env, newLoc)) {
                //std::cout << "BLOCKED" << "\n";
                return false;          
            }
        }

        return moveObjectTo(ref, env, newLoc);
    }

    bool addObjectTo(const TypeRef& ref, XYEnvironment& env, const XYLocation& loc) {
        auto it = env.xyspace.find(loc);
        if (it == env.xyspace.end()) return false;

        if (!registerObject(env.objects, ref)) {
            //std::cout << "UNREGISTERED" << "\n";
            return false;
        }

        it->second.emplace(ref);
        env.locations[ref.id] = loc;
        return true;
    }

    template <typename T>
    ObjectRef addObject(XYEnvironment& env, const XYLocation& loc) {
        T object;
        if (!addObjectTo(object.ref, env, loc)) return std::nullopt;
        return object.ref;
    }

    ObjectRef addAgent(XYEnvironment& env, const XYLocation& loc) { 
        return addObject<XYAgent>(env, loc); 
    }

    ObjectRef addWall(XYEnvironment& env, const XYLocation& loc) { 
        return addObject<Wall>(env, loc); 
    }

    ObjectRef addDirt(XYEnvironment& env, const XYLocation& loc) { 
        return addObject<Dirt>(env, loc); 
    }

    ObjectRef addGold(XYEnvironment& env, const XYLocation& loc) { 
        return addObject<Gold>(env, loc); 
    }


} // namespace environment 























