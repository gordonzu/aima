module;

#include <cstddef>
#include <stdexcept>
#include <variant>
#include <vector>
#include <map>

export module aima.environment;

namespace xyenv {
    export struct Wall{};
    export struct XYAgent{};

    export struct XYLocation {
        auto operator<=>(const XYLocation&) const = default;
        bool operator==(const XYLocation&) const = default;

        int x_ = 0;
        int y_ = 0;
    };

    using Object = std::variant<Wall, XYAgent>;
    using Vec = std::vector<Object>;
    using Map = std::map<XYLocation, Vec>;

    export struct XYEnvironment {
        unsigned w_  = 0;
        unsigned h_ = 0;

        Map map_;
    };

    export XYLocation makeXYLocation(int x, int y) {
        if (x <= 0 || y <= 0) throw std::invalid_argument("x and y must be > 0");

        XYLocation loc;
        loc.x_ = x;
        loc.y_ = y;

        return loc;
    }

    export XYEnvironment makeXYEnvironment(int w, int h) {
        if (w <= 0 || h <= 0) throw std::invalid_argument("width and height must be > 0");

        XYEnvironment env;
        env.w_ = static_cast<unsigned>(w);
        env.h_ = static_cast<unsigned>(h);

        
        for (int x = 1; x <= w; ++x) {
            for (int y = 1; y <= h; ++y) {
                env.map_.emplace(XYLocation(x,y), Vec()); 
            }
        }

        return env;
    }

    export size_t mapSize(const XYEnvironment& env) {
        return env.map_.size();
    }

    export std::size_t agentCount(const XYEnvironment& env, const XYLocation& loc) {
        auto it = env.map_.find(loc);
        if (it == env.map_.end()) return 0;
        
        return it->second.size();
    }

    export bool addAgentToLocation(const XYAgent& agent, 
                                   XYEnvironment& env, 
                                   const XYLocation& loc) {
        auto it = env.map_.find(loc);
        if (it == env.map_.end()) return false;
        else { 
            it->second.emplace_back(std::move(agent));
            return true;
        };           
    }

























} // namespace xyenv 














