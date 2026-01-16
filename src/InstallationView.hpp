#pragma once
#include <QWidget>
#include <QPainter>
#include <QPointF>
#include <QWheelEvent>
#include <QMouseEvent>
#include "Electrical.hpp"
#include "Geometry.hpp"
#include "Routing.hpp"
#include "Visualise.hpp"


class InstallationView : public QWidget {
    Q_OBJECT

public:
    explicit InstallationView(QWidget* parent = nullptr)
        : QWidget(parent) {
        setMouseTracking(true);
    }

    void setInstallation(const Installation* inst) {
        installation = inst;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        if (!installation) return;

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        p.translate(offset);
        p.scale(scale, scale);

        drawInstallation(p);
    }

    void wheelEvent(QWheelEvent* e) override {
        double delta = e->angleDelta().y() / 120.0;
        scale *= (1.0 + delta * 0.1);
        scale = std::clamp(scale, 0.1, 10.0);
        update();
    }

    void mousePressEvent(QMouseEvent* e) override {
        lastMousePos = e->pos();
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton) {
            QPointF delta = e->pos() - lastMousePos;
            offset += delta;
            lastMousePos = e->pos();
            update();
        }
    }

private:
    const Installation* installation = nullptr;

    double scale = 1.0;
    QPointF offset = QPointF(50, 50);
    QPointF lastMousePos;

    const double S = 80.0; // meters → pixels

    void drawInstallation(QPainter& p) {
        for (const auto& room : installation->rooms) {
            drawRoom(p, room);
        }
        for (const auto& route : installation->routes) {
            drawRoute(p, route);
        }
    }

    void drawRoom(QPainter& p, const Room& room) {
        QPointF origin(room.origin[0] * S, room.origin[1] * S);

        QRectF rect(origin.x(),
                    origin.y(),
                    room.dimensions.width_m * S,
                    room.dimensions.length_m * S);

        // Room outline
        p.setPen(QPen(Qt::gray, 1));
        p.drawRect(rect);

        // Room label
        p.setPen(Qt::blue);
        p.drawText(rect.topLeft() + QPointF(5, 15), QString::fromStdString(room.id));

        // Walls
        for (const auto& w : room.walls) {
            QPointF a = origin + QPointF(w.start[0] * S, w.start[1] * S);
            QPointF b = origin + QPointF(w.end[0] * S, w.end[1] * S);

            double thickness = w.thickness_m * (S / 0.15);

            QPen pen(Qt::black, thickness, Qt::SolidLine, Qt::RoundCap);
            p.setPen(pen);
            p.drawLine(a, b);

            // Openings
            for (const auto& op : w.openings) {
                double t = op.position * S;
                QPointF mid = a + (b - a) * (t / (S * std::hypot(w.end[0]-w.start[0], w.end[1]-w.start[1])));

                QColor c = (op.type == "door") ? QColor("#AA5500") : QColor("#00AACC");
                p.setBrush(c);
                p.setPen(Qt::NoPen);
                p.drawEllipse(mid, 5, 5);
            }
        }

        // Devices
        for (const auto& d : room.devices) {
            QPointF pos = origin + QPointF(d.position[0] * S, d.position[1] * S);
            drawDevice(p, d, pos);
        }
    }

    void drawDevice(QPainter& p, const Device& d, const QPointF& pos) {
        if (d.type == "socket") {
            p.setBrush(QColor("#FFCC88"));
            p.setPen(QPen(QColor("#994400"), 1));
            p.drawRect(QRectF(pos.x()-4, pos.y()-4, 8, 8));
        }
        else if (d.type == "switch") {
            p.setBrush(QColor("#00AA00"));
            p.setPen(QPen(QColor("#004400"), 1));
            QPolygonF tri;
            tri << QPointF(pos.x(), pos.y()-5)
                << QPointF(pos.x()+5, pos.y()+5)
                << QPointF(pos.x()-5, pos.y()+5);
            p.drawPolygon(tri);
        }
        else if (d.type == "light") {
            p.setBrush(QColor("#FFFF66"));
            p.setPen(Qt::NoPen);
            p.drawEllipse(pos, 6, 6);
        }
        else if (d.type == "junction_box") {
            p.setBrush(QColor("#CCCCFF"));
            p.setPen(QPen(QColor("#444488"), 1));
            p.drawRect(QRectF(pos.x()-5, pos.y()-5, 10, 10));
        }
        else {
            p.setBrush(Qt::red);
            p.drawEllipse(pos, 4, 4);
        }
    }

    void drawRoute(QPainter& p, const WireRoute& route) {
        for (const auto& seg : route.segments) {
            const Room* room = nullptr;
            for (const auto& r : installation->rooms)
                if (r.id == seg.room_id)
                    room = &r;

            if (!room) continue;

            QPointF origin(room->origin[0] * S, room->origin[1] * S);

            QPointF a = origin + QPointF(seg.start[0] * S, seg.start[1] * S);
            QPointF b = origin + QPointF(seg.end[0] * S, seg.end[1] * S);

            QColor c(routeColor(seg.type).c_str());
            p.setPen(QPen(c, 2));
            p.drawLine(a, b);
        }
    }
};
