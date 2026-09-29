module;

#include <cstddef>
#include <stdexcept>
#include <variant>
#include <vector>
#include <map>
#include <optional>

export module aima.environment;

namespace xyenv {

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
        XYLocation(int x, int y) : x_(x), y_(y) {
            if (x <= 0 || y <= 0) throw std::invalid_argument("params must be > 0");
        }

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

    export bool addAgentToLocation(const XYAgent& agent, 
                                   XYEnvironment& env, 
                                   const XYLocation& loc) {
        auto it = env.map_.find(loc);
        if (it == env.map_.end()) return false;
        if (it->second.has_value()) return false;
        it->second = Object(std::move(agent));
        return true;
    }

























} // namespace xyenv 














