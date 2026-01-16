// Electrical.hpp

#pragma once
#include <string>
#include <vector>

#include "Routing.hpp"
#include "Geometry.hpp"

struct Environment {
    double ambientTemperatureC;
    std::string installationMethod;
    double groupingFactor;
};

struct Cable {
    std::string material;          // "copper" | "aluminium"
    double crossSectionMm2;
    int cores;
    std::string insulation;        // "PVC", "XLPE", ...
};

struct Protection {
    std::string deviceType;        // "MCB", "Fuse", "RCD", ...
    double ratedCurrentA;
    std::string curve;             // "B", "C", ...
    double breakingCapacityKA;
};

struct Circuit {
    std::string id;
    std::string type;              // "lighting", "sockets", ...
    std::string phaseType;         // "single_phase", "three_phase"
    double designPowerW;
    double nominalVoltage;
    double lengthM;
    double maxAllowedVoltageDropPercent;
    Environment environment;
    Cable cable;
    Protection protection;
};

struct Installation {
    std::string name;
    std::string standard;
    double supplyVoltage;
    std::vector<Circuit> circuits;
    std::vector<Room> rooms;
    std::vector<WireRoute> routes;
};

