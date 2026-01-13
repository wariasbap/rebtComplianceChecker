// JsonAdapters.hpp
#pragma once
#include <nlohmann/json.hpp>
#include "Geometry.hpp"
#include "Routing.hpp"
#include "Electrical.hpp"

using nlohmann::json;

// --- Geometry ---

inline void from_json(const json& j, Opening& o) {
    j.at("type").get_to(o.type);
    j.at("position").get_to(o.position);
    j.at("width_m").get_to(o.width_m);
    j.at("height_m").get_to(o.height_m);
}

inline void from_json(const json& j, Wall& w) {
    j.at("id").get_to(w.id);
    j.at("start").get_to(w.start);
    j.at("end").get_to(w.end);
    j.at("thickness_m").get_to(w.thickness_m);
    if (j.contains("openings")) j.at("openings").get_to(w.openings);
}

inline void from_json(const json& j, Device& d) {
    j.at("id").get_to(d.id);
    j.at("type").get_to(d.type);
    j.at("position").get_to(d.position);
}

inline void from_json(const json& j, RoomDimensions& rd) {
    j.at("width_m").get_to(rd.width_m);
    j.at("length_m").get_to(rd.length_m);
    j.at("height_m").get_to(rd.height_m);
}

inline void from_json(const json& j, Room& r) {
    j.at("id").get_to(r.id);
    j.at("origin").get_to(r.origin);
    j.at("dimensions").get_to(r.dimensions);
    j.at("walls").get_to(r.walls);
    j.at("devices").get_to(r.devices);
}

// --- Routing ---

inline void from_json(const json& j, RouteSegment& s) {
    j.at("type").get_to(s.type);
    if (j.contains("room_id")) j.at("room_id").get_to(s.room_id);
    if (j.contains("wall_id")) j.at("wall_id").get_to(s.wall_id);
    j.at("start").get_to(s.start);
    j.at("end").get_to(s.end);
    if (j.contains("depth_mm")) j.at("depth_mm").get_to(s.depth_mm);
}

inline void from_json(const json& j, WireRoute& r) {
    j.at("id").get_to(r.id);
    j.at("circuit_id").get_to(r.circuit_id);
    j.at("start_device").get_to(r.start_device);
    j.at("end_device").get_to(r.end_device);
    j.at("segments").get_to(r.segments);
}

// --- Electrical ---

inline void from_json(const json& j, Environment& e) {
    j.at("ambient_temperature_c").get_to(e.ambientTemperatureC);
    j.at("installation_method").get_to(e.installationMethod);
    j.at("grouping_factor").get_to(e.groupingFactor);
}

inline void from_json(const json& j, Cable& c) {
    j.at("material").get_to(c.material);
    j.at("cross_section_mm2").get_to(c.crossSectionMm2);
    j.at("cores").get_to(c.cores);
    j.at("insulation").get_to(c.insulation);
}

inline void from_json(const json& j, Protection& p) {
    j.at("device_type").get_to(p.deviceType);
    j.at("rated_current_a").get_to(p.ratedCurrentA);
    j.at("curve").get_to(p.curve);
    j.at("breaking_capacity_ka").get_to(p.breakingCapacityKA);
}

inline void from_json(const json& j, Circuit& c) {
    j.at("id").get_to(c.id);
    j.at("type").get_to(c.type);
    j.at("phase_type").get_to(c.phaseType);
    j.at("design_power_w").get_to(c.designPowerW);
    j.at("nominal_voltage").get_to(c.nominalVoltage);
    j.at("length_m").get_to(c.lengthM);
    j.at("max_allowed_voltage_drop_percent").get_to(c.maxAllowedVoltageDropPercent);
    j.at("environment").get_to(c.environment);
    j.at("cable").get_to(c.cable);
    j.at("protection").get_to(c.protection);
}

inline void from_json(const json& j, Installation& inst) {
    const auto& jin = j.at("installation");
    jin.at("name").get_to(inst.name);
    jin.at("standard").get_to(inst.standard);
    jin.at("supply_voltage").get_to(inst.supplyVoltage);
    if (jin.contains("circuits")) jin.at("circuits").get_to(inst.circuits);
    if (jin.contains("rooms")) jin.at("rooms").get_to(inst.rooms);
    if (jin.contains("routes")) jin.at("routes").get_to(inst.routes);
}

