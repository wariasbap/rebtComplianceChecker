// Routing.hpp
#pragma once
#include <string>
#include <vector>
#include <array>

struct RouteSegment {
    std::string type;             // "wall_chase", "ceiling", ...
    std::string room_id;          // optional (may be empty)
    std::string wall_id;          // optional
    std::array<double, 3> start;  // room frame
    std::array<double, 3> end;    // room frame
    double depth_mm = 0.0;        // optional
};

struct WireRoute {
    std::string id;
    std::string circuit_id;
    std::string start_device;
    std::string end_device;
    std::vector<RouteSegment> segments;
};

