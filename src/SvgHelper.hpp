// SvgHelper.hpp
#pragma once
#include <string>
#include <sstream>
#include <vector>
#include "Geometry.hpp"
#include "Routing.hpp"

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
           << "\" font-size=\"" << size << "\">"
           << text << "</text>\n";
    }

    std::string build() const {
        std::ostringstream out;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
            << "width=\"" << width << "\" height=\"" << height << "\">\n"
            << ss.str()
            << "</svg>\n";
        return out.str();
    }

private:
    double width, height;
    std::ostringstream ss;
};

