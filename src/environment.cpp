module;

#include <cstddef>
#include <stdexcept>
#include <variant>
#include <map>
#include <optional>
#include <iostream>

export module aima.environment;

namespace environment {
    
    //////////////////// data //////////////////////

    export enum class Type { wall, xyagent };

    export struct TypeRef { 
        Type type_; 
        int id_; 

        explicit TypeRef(Type t) : type_(t), id_(nextId()) {}
        TypeRef() = delete;

    private:
        static int nextId() {
            static int counter = 0;
            return ++counter;
        }
    };

    export struct Wall{ 
      TypeRef ref_;
      Wall() : ref_(Type::wall) {}
    };

    export struct XYAgent{ 
      TypeRef ref_;
      XYAgent() : ref_(Type::xyagent) {}
    };

    export struct XYLocation {
        int x_ = 0;
        int y_ = 0;

        XYLocation() = default;
        XYLocation(int x, int y) : x_(x), y_(y) {}

        auto operator<=>(const XYLocation&) const = default;
        bool operator==(const XYLocation&) const = default;

    };

    using Object = std::variant<Wall, XYAgent>;
    using Tile = std::optional<Object>;
    using Map = std::map<XYLocation, Tile>;

    export struct XYEnvironment {
        unsigned w_  = 0;
        unsigned h_ = 0;
        Map map_;
    };

    /////////////////// logic ///////////////////////////////  

    export std::ostream& operator<<(std::ostream& os, const TypeRef& t) {
        const char* name = (t.type_ == Type::xyagent) ? "xyagent" : "wall";
        return os << "[" << name << ", " << t.id_ << "]";
    }

    export std::ostream& operator<<(std::ostream& os, const std::optional<TypeRef>& t) {
        if (!t) return os << "[none]";
        return os << *t;
    }

    export bool inBounds(const XYEnvironment& env, const XYLocation& loc) {
        return loc.x_ >= 1 && loc.y_ >= 1 &&
               loc.x_ <= static_cast<int>(env.w_) &&
               loc.y_ <= static_cast<int>(env.h_);
    }

    export bool addAgentToLocation(XYAgent agent, XYEnvironment& env, const XYLocation& loc) {
        auto it = env.map_.find(loc);
        if (it == env.map_.end()) return false;
        if (it->second.has_value()) return false;
        it->second = Object(std::move(agent));
        return true;
    }

    export std::optional<TypeRef> getObjectAt(const XYEnvironment& env, const XYLocation& loc) {
        if (!inBounds(env, loc)) {
            std::cout << "location out of bounds..." << '\n';
            return std::nullopt;
        }

        auto it = env.map_.find(loc);

        if (it == env.map_.end()) {
            std::cout << "location not found in map..." << '\n';
            return std::nullopt;
        }

        const Tile& tile = it->second;

        if (!tile.has_value()) {
              std::cout << "Tile has no value..." << '\n';
              return std::nullopt;
        }

        const Object& obj = tile.value();

        return std::visit(
            [](const auto& x) -> TypeRef {
                return x.ref_;
            },
            obj
        );
    }
    
    export XYEnvironment makeXYEnvironment(int w, int h) {
        if (w <= 0 || h <= 0) throw std::invalid_argument("params must be > 0");

        XYEnvironment env;
        env.w_ = static_cast<unsigned>(w);
        env.h_ = static_cast<unsigned>(h);

        
        for (int x = 1; x <= w; ++x) {
            for (int y = 1; y <= h; ++y) {
                env.map_.emplace(XYLocation(x,y), std::nullopt); 
            }
        }

        return env;
    }

    export size_t mapSize(const XYEnvironment& env) {
        return env.map_.size();
    }

    export bool isOccupied(const XYEnvironment& env, const XYLocation& loc) {
        auto it = env.map_.find(loc);
        return it != env.map_.end() && it->second.has_value(); 
    }

} // namespace environment 














