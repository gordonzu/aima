module;

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <variant>
#include <map>
#include <optional>

export module aima.environment;

namespace env {
    
    //////////////////// data //////////////////////

    export enum class Type { wall, xyagent };

    export struct TypeRef { 
        Type type; 
        int id; 

        TypeRef() = default;
        TypeRef(Type t, int i) : type(t), id(i) {}
    };

    export struct Wall{ 
      int id_=0;
      Wall() = default;
      explicit Wall(int id) : id_(id) {}
    };

    export struct XYAgent{ 
      int id_=0;
      XYAgent() = default;
      explicit XYAgent(int id) : id_(id) {}
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

    export bool inBounds(const XYEnvironment& env, const XYLocation& loc) {
        return loc.x_ >= 1 && loc.y_ >= 1 &&
               loc.x_ <= static_cast<int>(env.w_) &&
               loc.y_ <= static_cast<int>(env.h_);
    }

    export std::optional<TypeRef> getObjectAt(const XYEnvironment& env, const XYLocation& loc) {
        if (!inBounds(env, loc)) return std::nullopt;

        auto it = env.map_.find(loc);
        if (it == env.map_.end()) return std::nullopt;

        const Tile& tile = it->second;
        if (!tile.has_value()) return std::nullopt;

        const Object& obj = tile.value();

        return std::visit(
            [](const auto& x) -> TypeRef {
                using T = std::decay_t<decltype(x)>;
                if constexpr (std::is_same_v<T, Wall> ) {
                    return TypeRef(Type::wall, x.id_);
                } else if constexpr (std::is_same_v<T, XYAgent>) {
                    return TypeRef(Type::xyagent, x.id_);
                }
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

    export bool addAgentToLocation(XYAgent agent, XYEnvironment& env, const XYLocation& loc) {
        auto it = env.map_.find(loc);
        if (it == env.map_.end()) return false;
        if (it->second.has_value()) return false;
        it->second = Object(std::move(agent));
        return true;
    }

} // namespace env 














