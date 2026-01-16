#pragma once
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QPen>
#include <QBrush>

#include "Electrical.hpp"
#include "Geometry.hpp"
#include "Routing.hpp"

class InstallationScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit InstallationScene(QObject* parent = nullptr)
        : QGraphicsScene(parent) {}

    void setInstallation(const Installation* inst) {
        installation = inst;
        rebuild();
    }

signals:
    void elementSelected(QString type, QString id);

private:
    const Installation* installation = nullptr;
    const double S = 80.0; // meters → pixels

    void rebuild() {
        clear();
        if (!installation) return;

        for (const auto& room : installation->rooms)
            addRoom(room);

        for (const auto& route : installation->routes)
            addRoute(route);
    }

    // ---------------------- Rooms ----------------------

    void addRoom(const Room& room) {
        QPointF origin(room.origin[0] * S, room.origin[1] * S);

        // Room outline
        addRect(origin.x(),
                origin.y(),
                room.dimensions.width_m * S,
                room.dimensions.length_m * S,
                QPen(Qt::gray));

        // Room label
        addText(QString::fromStdString(room.id))
            ->setPos(origin + QPointF(5, 5));

        // Walls
        for (const auto& w : room.walls) {
            QPointF a = origin + QPointF(w.start[0] * S, w.start[1] * S);
            QPointF b = origin + QPointF(w.end[0] * S, w.end[1] * S);

            double thickness = w.thickness_m * (S / 0.15);

            auto* line = addLine(QLineF(a, b),
                                 QPen(Qt::black, thickness));
            line->setData(0, "wall");
            line->setData(1, QString::fromStdString(w.id));
        }

        // Devices
        for (const auto& d : room.devices) {
            QPointF pos = origin + QPointF(d.position[0] * S, d.position[1] * S);

            auto* item = addDevice(d, pos);
            item->setData(0, "device");
            item->setData(1, QString::fromStdString(d.id));
        }
    }

    QGraphicsItem* addDevice(const Device& d, const QPointF& pos) {
        if (d.type == "light") {
            auto* e = addEllipse(pos.x()-6, pos.y()-6, 12, 12,
                                 QPen(Qt::yellow), QBrush(Qt::yellow));
            return e;
        }
        if (d.type == "switch") {
            QPolygonF tri;
            tri << QPointF(pos.x(), pos.y()-5)
                << QPointF(pos.x()+5, pos.y()+5)
                << QPointF(pos.x()-5, pos.y()+5);
            auto* p = addPolygon(tri, QPen(Qt::green), QBrush(Qt::green));
            return p;
        }
        if (d.type == "socket") {
            auto* r = addRect(pos.x()-4, pos.y()-4, 8, 8,
                              QPen(Qt::darkYellow), QBrush(Qt::yellow));
            return r;
        }

        // fallback
        return addEllipse(pos.x()-4, pos.y()-4, 8, 8,
                          QPen(Qt::red), QBrush(Qt::red));
    }

    // ---------------------- Routes ----------------------

    void addRoute(const WireRoute& route) {
        for (const auto& seg : route.segments) {
            const Room* room = nullptr;
            for (const auto& r : installation->rooms)
                if (r.id == seg.room_id)
                    room = &r;

            if (!room) continue;

            QPointF origin(room->origin[0] * S, room->origin[1] * S);

            QPointF a = origin + QPointF(seg.start[0] * S, seg.start[1] * S);
            QPointF b = origin + QPointF(seg.end[0] * S, seg.end[1] * S);

            auto* line = addLine(QLineF(a, b),
                                 QPen(routeColor(seg.type), 2));
            line->setData(0, "route");
            line->setData(1, QString::fromStdString(route.id));
        }
    }

    QColor routeColor(const std::string& type) {
        if (type == "ceiling") return QColor("#00CC44");
        if (type == "wall_chase") return QColor("#FF8800");
        if (type == "vertical_drop") return QColor("#FF0000");
        if (type == "conduit") return QColor("#0066FF");
        return QColor("#000000");
    }
};
