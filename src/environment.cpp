module;

#include <cstddef>
#include <cstdio>
#include <stdexcept>
#include <variant>
#include <vector>

export module aima.environment;

namespace xyenv {
    export struct Wall{};
    export struct XYAgent{};
    export struct XYLocation{};

    using Object = std::variant<Wall, XYAgent>;

    export struct XYEnvironment {
        unsigned width  = 0;
        unsigned height = 0;

        std::vector<std::vector<Object>> objects;
    };

    export XYEnvironment make_xy_environment(int w, int h)
    {
        if (w <= 0 || h <= 0) throw std::invalid_argument("width and height must be > 0");
        XYEnvironment env;
        env.width   = w;
        env.height  = h;
        env.objects.resize(static_cast<size_t>(w * h));
        return env;
    }

    export size_t getMapSize(const XYEnvironment& xy) 
    {
        return xy.objects.size();
    }


} // namespace env 

