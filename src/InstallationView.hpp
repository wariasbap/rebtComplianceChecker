#pragma once
#include <QWidget>
#include <QPointF>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include <optional>

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

signals:
    void elementSelected(QString type, QString id);

protected:
    void paintEvent(QPaintEvent*) override {
        if (!installation) return;

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        p.translate(offset);
        p.scale(scale, scale);

        drawInstallation(p);
        drawHoverHighlight(p);
        drawSelectionHighlight(p);
    }

    void wheelEvent(QWheelEvent* e) override {
        double delta = e->angleDelta().y() / 120.0;
        scale *= (1.0 + delta * 0.1);
        scale = std::clamp(scale, 0.1, 10.0);
        update();
    }

    void mousePressEvent(QMouseEvent* e) override {
        lastMousePos = e->pos();

        if (e->button() == Qt::LeftButton) {
            auto hit = hitTest(e->pos());
            if (hit.has_value()) {
                selected = hit;
                emit elementSelected(hit->type, hit->id);
            } else {
                selected.reset();
            }
            update();
        }
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton) {
            QPointF delta = e->pos() - lastMousePos;
            offset += delta;
            lastMousePos = e->pos();
            update();
            return;
        }

        // Hover detection
        hover = hitTest(e->pos());
        update();

        if (hover.has_value()) {
            QToolTip::showText(
                e->globalPosition().toPoint(),
                hover->tooltip
            );
        } else {
            QToolTip::hideText();
        }
    }

private:
    struct HitInfo {
        QString type;     // "device", "wall", "route"
        QString id;
        QString tooltip;
        QPointF p1, p2;   // for drawing highlight
    };

    const Installation* installation = nullptr;

    double scale = 1.0;
    QPointF offset = QPointF(50, 50);
    QPointF lastMousePos;

    const double S = 80.0; // meters → pixels

    std::optional<HitInfo> hover;
    std::optional<HitInfo> selected;

    // ---------------------- Drawing ----------------------

    void drawInstallation(QPainter& p) {
        for (const auto& room : installation->rooms)
            drawRoom(p, room);

        for (const auto& route : installation->routes)
            drawRoute(p, route);
    }

    void drawRoom(QPainter& p, const Room& room) {
        QPointF origin(room.origin[0] * S, room.origin[1] * S);

        QRectF rect(origin.x(),
                    origin.y(),
                    room.dimensions.width_m * S,
                    room.dimensions.length_m * S);

        p.setPen(QPen(Qt::gray, 1));
        p.drawRect(rect);

        p.setPen(Qt::blue);
        p.drawText(rect.topLeft() + QPointF(5, 15),
                   QString::fromStdString(room.id));

        // Walls
        for (const auto& w : room.walls) {
            QPointF a = origin + QPointF(w.start[0] * S, w.start[1] * S);
            QPointF b = origin + QPointF(w.end[0] * S, w.end[1] * S);

            double thickness = w.thickness_m * (S / 0.15);

            p.setPen(QPen(Qt::black, thickness, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(a, b);
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

    // ---------------------- Hit Testing ----------------------

    std::optional<HitInfo> hitTest(const QPointF& mousePos) {
        if (!installation) return std::nullopt;

        QPointF world = (mousePos - offset) / scale;

        // Devices
        for (const auto& room : installation->rooms) {
            QPointF origin(room.origin[0] * S, room.origin[1] * S);

            for (const auto& d : room.devices) {
                QPointF pos = origin + QPointF(d.position[0] * S, d.position[1] * S);

                if (QLineF(pos, world).length() < 10) {
                    return HitInfo{
                        "device",
                        QString::fromStdString(d.id),
                        QString("Device %1 (%2)")
                            .arg(QString::fromStdString(d.id))
                            .arg(QString::fromStdString(d.type)),
                        pos, pos
                    };
                }
            }
        }

        // Routes
        for (const auto& route : installation->routes) {
            for (const auto& seg : route.segments) {
                const Room* room = nullptr;
                for (const auto& r : installation->rooms)
                    if (r.id == seg.room_id)
                        room = &r;

                if (!room) continue;

                QPointF origin(room->origin[0] * S, room->origin[1] * S);

                QPointF a = origin + QPointF(seg.start[0] * S, seg.start[1] * S);
                QPointF b = origin + QPointF(seg.end[0] * S, seg.end[1] * S);

	if (distancePointToSegment(world, a, b) < 5.0) {
                    return HitInfo{
                        "route",
                        QString::fromStdString(route.id),
                        QString("Route %1 (%2)")
                            .arg(QString::fromStdString(route.id))
                            .arg(QString::fromStdString(seg.type)),
                        a, b
                    };
                }
            }
        }

        return std::nullopt;
    }

    // ---------------------- Highlights ----------------------

    void drawHoverHighlight(QPainter& p) {
        if (!hover.has_value()) return;

        p.setPen(QPen(Qt::yellow, 3, Qt::DashLine));
        p.drawLine(hover->p1, hover->p2);
    }

    void drawSelectionHighlight(QPainter& p) {
        if (!selected.has_value()) return;

        p.setPen(QPen(Qt::red, 4));
        p.drawLine(selected->p1, selected->p2);
    }
};
