// Geometry.hpp
#pragma once
#include <string>
#include <vector>
#include <array>

struct Opening {
    std::string type;        // "window" | "door"
    double position;         // along wall length (m)
    double width_m;
    double height_m;
};

struct Wall {
    std::string id;
    std::array<double, 3> start;   // room frame
    std::array<double, 3> end;     // room frame
    double thickness_m;
    std::vector<Opening> openings;
};

struct Device {
    std::string id;
    std::string type;              // "socket", "switch", "light", ...
    std::array<double, 3> position; // room frame
};

struct RoomDimensions {
    double width_m;
    double length_m;
    double height_m;
};

struct Room {
    std::string id;
    std::array<double, 3> origin; // house frame
    RoomDimensions dimensions;
    std::vector<Wall> walls;
    std::vector<Device> devices;
};

