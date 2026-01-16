#pragma once
#include <fstream>
#include <cmath>
#include <sstream>
#include <vector>
#include <string>

#include <QPointF>

#include "Geometry.hpp"
#include "Routing.hpp"
#include "Electrical.hpp"

// ---------------------- SvgBuilder ----------------------

class SvgBuilder {
public:
    SvgBuilder(double width, double height)
        : width(width), height(height) {}

    void addLine(double x1, double y1, double x2, double y2,
                 const std::string& color = "black",
                 double stroke = 2.0) {
        ss << "<line x1=\"" << x1 << "\" y1=\"" << y1
           << "\" x2=\"" << x2 << "\" y2=\"" << y2
           << "\" stroke=\"" << color
           << "\" stroke-width=\"" << stroke << "\" />\n";
    }

    void addCircle(double cx, double cy, double r,
                   const std::string& color = "red") {
        ss << "<circle cx=\"" << cx << "\" cy=\"" << cy
           << "\" r=\"" << r
           << "\" fill=\"" << color << "\" />\n";
    }

    void addText(double x, double y, const std::string& text,
                 const std::string& color = "black",
                 int size = 12) {
        ss << "<text x=\"" << x << "\" y=\"" << y
           << "\" fill=\"" << color
           << "\" font-size=\"" << size
           << "\" dominant-baseline=\"middle\""
           << " text-anchor=\"start\">"
           << text << "</text>\n";
    }

    void addRect(double x, double y, double w, double h,
                 const std::string& color = "none",
                 const std::string& stroke = "black",
                 double stroke_width = 1.0,
                 double opacity = 1.0) {
        ss << "<rect x=\"" << x << "\" y=\"" << y
           << "\" width=\"" << w << "\" height=\"" << h
           << "\" fill=\"" << color
           << "\" stroke=\"" << stroke
           << "\" stroke-width=\"" << stroke_width
           << "\" fill-opacity=\"" << opacity << "\" />\n";
    }

    void addPolygon(const std::vector<std::pair<double,double>>& pts,
                    const std::string& color = "#444",
                    const std::string& stroke = "none",
                    double stroke_width = 1.0,
                    double opacity = 1.0) {
        ss << "<polygon points=\"";
        for (const auto& p : pts) {
            ss << p.first << "," << p.second << " ";
        }
        ss << "\" fill=\"" << color
           << "\" stroke=\"" << stroke
           << "\" stroke-width=\"" << stroke_width
           << "\" fill-opacity=\"" << opacity << "\" />\n";
    }

    std::string build() const {
        std::ostringstream out;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
            << "width=\"" << width << "\" height=\"" << height << "\">\n"
            << "<rect width=\"100%\" height=\"100%\" fill=\"white\" />\n"
            << ss.str()
            << "</svg>\n";
        return out.str();
    }

private:
    double width, height;
    std::ostringstream ss;
};

// ---------------------- helpers ----------------------

static double distancePointToSegment(const QPointF& p,
                                     const QPointF& a,
                                     const QPointF& b)
{
    // Vector AB
    double ABx = b.x() - a.x();
    double ABy = b.y() - a.y();

    // Vector AP
    double APx = p.x() - a.x();
    double APy = p.y() - a.y();

    double ab2 = ABx*ABx + ABy*ABy;
    if (ab2 == 0.0)
        return std::hypot(APx, APy); // a == b

    // Project AP onto AB, clamp t to [0,1]
    double t = (APx*ABx + APy*ABy) / ab2;
    t = std::max(0.0, std::min(1.0, t));

    // Closest point on segment
    double cx = a.x() + t * ABx;
    double cy = a.y() + t * ABy;

    // Distance PC
    return std::hypot(p.x() - cx, p.y() - cy);
}


inline std::string routeColor(const std::string& type) {
    if (type == "wall_chase")      return "#FF8800";
    if (type == "ceiling")         return "#00CC44";
    if (type == "floor")           return "#AA7744";
    if (type == "conduit")         return "#0066FF";
    if (type == "free_air")        return "#AA00FF";
    if (type == "vertical_drop")   return "#FF0000";
    if (type == "horizontal_run")  return "#00FFFF";
    return "#000000";
}

inline void addWallThick(SvgBuilder& svg,
                         double x1, double y1,
                         double x2, double y2,
                         double thickness_px,
                         const std::string& color = "#444") {
    double dx = x2 - x1;
    double dy = y2 - y1;
    double len = std::hypot(dx, dy);
    if (len == 0.0) return;

    double nx = -dy / len * thickness_px / 2.0;
    double ny =  dx / len * thickness_px / 2.0;

    svg.addPolygon({
        {x1 + nx, y1 + ny},
        {x2 + nx, y2 + ny},
        {x2 - nx, y2 - ny},
        {x1 - nx, y1 - ny}
    }, color, "#222", 0.5);
}

inline void addOpeningMarker(SvgBuilder& svg,
                             double x, double y,
                             double width_px,
                             const std::string& type) {
    std::string color = (type == "door") ? "#AA5500" : "#00AACC";
    svg.addRect(x - width_px / 2.0, y - 3.0, width_px, 6.0,
                color, "none", 0.0, 0.9);
}

// very simple device icon renderer
inline void drawDevice(SvgBuilder& svg,
                       const Device& d,
                       double x, double y) {
    if (d.type == "socket") {
        svg.addRect(x - 4, y - 4, 8, 8, "#FFCC88", "#994400", 1.0);
    } else if (d.type == "switch") {
        svg.addPolygon({{x, y-5}, {x+5, y+5}, {x-5, y+5}},
                       "#00AA00", "#004400", 1.0);
    } else if (d.type == "light") {
        svg.addCircle(x, y, 6, "#FFFF66");
    } else if (d.type == "junction_box") {
        svg.addRect(x-5, y-5, 10, 10, "#CCCCFF", "#444488", 1.0);
        svg.addLine(x-5, y-5, x+5, y+5, "#444", 1);
        svg.addLine(x+5, y-5, x-5, y+5, "#444", 1);
    } else if (d.type == "distribution_board") {
        svg.addRect(x-8, y-6, 16, 12, "#FFFFFF", "#000000", 1.5);
    } else {
        svg.addCircle(x, y, 4, "#FF0000");
    }
}

// ---------------------- main visualiser ----------------------

inline std::string visualiseInstallation(const Installation& inst) {
    // size – tweak as needed
    SvgBuilder svg(1600, 1200);

    // meters → pixels scale
    const double S = 80.0;
    const double wallThicknessMetersDefault = 0.15;
    const double wallThicknessPxBase = 6.0;

    // draw rooms, walls, forbidden zones, devices
    for (const auto& room : inst.rooms) {
        double ox = room.origin[0] * S;
        double oy = room.origin[1] * S;

        // room outline (thin)
        svg.addRect(ox, oy,
                    room.dimensions.width_m * S,
                    room.dimensions.length_m * S,
                    "none", "#CCCCCC", 1.0);

        svg.addText(ox + 5, oy + 15, room.id, "blue", 14);

        // room origin marker
        svg.addCircle(ox, oy, 3, "#0000FF");
        svg.addText(ox + 5, oy - 5, "origin", "#0000FF", 10);

        // simple forbidden zones demo (REBT-like)
        // horizontal forbidden band at 0.30–0.50 m above floor
        double hz_y = oy + 0.30 * S;
        double hz_h = 0.20 * S;
        svg.addRect(ox, hz_y,
                    room.dimensions.width_m * S,
                    hz_h,
                    "#FF0000", "none", 0.2, 0.15);

        // vertical forbidden band near left wall (0–0.15 m)
        double vz_w = 0.15 * S;
        svg.addRect(ox, oy,
                    vz_w, room.dimensions.length_m * S,
                    "#FF0000", "none", 0.2, 0.15);

        // walls
        for (const auto& w : room.walls) {
            double x1 = ox + w.start[0] * S;
            double y1 = oy + w.start[1] * S;
            double x2 = ox + w.end[0] * S;
            double y2 = oy + w.end[1] * S;

            double wallThicknessMeters = (w.thickness_m > 0.0)
                                         ? w.thickness_m
                                         : wallThicknessMetersDefault;
            double wallThicknessPx =
                wallThicknessPxBase * (wallThicknessMeters / wallThicknessMetersDefault);

            addWallThick(svg, x1, y1, x2, y2, wallThicknessPx, "#555555");

            // openings: project along wall direction
            double dx = x2 - x1;
            double dy = y2 - y1;
            double len = std::hypot(dx, dy);
            if (len == 0.0) continue;
            double ux = dx / len;
            double uy = dy / len;

            for (const auto& op : w.openings) {
                double px = x1 + ux * (op.position * S);
                double py = y1 + uy * (op.position * S);
                double opWidthPx = op.width_m * S;
                addOpeningMarker(svg, px, py, opWidthPx, op.type);
            }
        }

        // devices
        for (const auto& d : room.devices) {
            double dx = ox + d.position[0] * S;
            double dy = oy + d.position[1] * S;

            drawDevice(svg, d, dx, dy);
            svg.addText(dx + 8, dy, d.id, "#444444", 10);
        }
    }

    // wire routes
    for (const auto& route : inst.routes) {
        for (const auto& seg : route.segments) {
            const Room* roomPtr = nullptr;
            for (const auto& r : inst.rooms) {
                if (r.id == seg.room_id) {
                    roomPtr = &r;
                    break;
                }
            }
            if (!roomPtr) continue;

            double ox = roomPtr->origin[0] * S;
            double oy = roomPtr->origin[1] * S;

            double x1 = ox + seg.start[0] * S;
            double y1 = oy + seg.start[1] * S;
            double x2 = ox + seg.end[0] * S;
            double y2 = oy + seg.end[1] * S;

            svg.addLine(x1, y1, x2, y2, routeColor(seg.type), 2.0);
        }
    }

    return svg.build();
}

// optional convenience function to write directly to file
inline void writeInstallationSvg(const Installation& inst,
                                 const std::string& filename) {
    std::ofstream out(filename);
    out << visualiseInstallation(inst);
}

