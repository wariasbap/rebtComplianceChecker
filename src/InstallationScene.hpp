#pragma once
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QObject>
#include <QMenu>
#include <QAction>
#include <QPen>
#include <QBrush>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <cmath>

#include "Geometry.hpp"
#include "Electrical.hpp"
#include "Routing.hpp"

// ---------------------- Layer control ----------------------

struct Layers {
    bool walls       = true;
    bool devices     = true;
    bool routes      = true;
    bool roomLabels  = true;
    bool grid        = true;
};

// ---------------------- Helpers ----------------------

inline QColor routeColor(const std::string& type) {
    if (type == "ceiling")        return QColor("#00CC44");
    if (type == "wall_chase")     return QColor("#FF8800");
    if (type == "vertical_drop")  return QColor("#FF0000");
    if (type == "conduit")        return QColor("#0066FF");
    if (type == "free_air")       return QColor("#AA00FF");
    return QColor("#000000");
}

inline QPointF snapToGrid(const QPointF& p, double grid) {
    return QPointF(
        std::round(p.x() / grid) * grid,
        std::round(p.y() / grid) * grid
    );
}

// ============================================================================
// DEVICE ITEM  (QObject + QGraphicsEllipseItem)
// ============================================================================

class DeviceItem :
    public QObject,
    public QGraphicsEllipseItem
{
    Q_OBJECT

public:
    DeviceItem(Room* room,
               Device* dev,
               const QPointF& posPx,
               double scaleMetersToPx,
               double gridPx,
               QGraphicsItem* parent = nullptr)
        : QObject(),
          QGraphicsEllipseItem(parent),
          m_room(room),
          m_device(dev),
          S(scaleMetersToPx),
          grid(gridPx)
    {
        setRect(-6, -6, 12, 12);
        setBrush(Qt::yellow);
        setPen(QPen(Qt::darkYellow, 1));
        setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges);
        setAcceptHoverEvents(true);
        setPos(posPx);
    }

    Room* room() const { return m_room; }
    Device* deviceModel() const { return m_device; }

signals:
    void deviceMovedMeters(QString id, QPointF newPosMeters);
    void deviceSelected(QString type, QString id);
    void deviceSelectedDetailed(QString id, QString type, QPointF posMeters, double zMeters);
    

protected:
    QVariant itemChange(GraphicsItemChange change,
                        const QVariant& value) override
    {
        if (change == ItemPositionChange && scene()) {
            QPointF p = value.toPointF();
            return snapToGrid(p, grid);
        }

        if (change == ItemPositionHasChanged && scene() && m_room && m_device) {
            QPointF p = pos();
            QPointF origin(m_room->origin[0] * S, m_room->origin[1] * S);
            QPointF localPx = p - origin;
            QPointF localMeters(localPx.x() / S, localPx.y() / S);

            m_device->position[0] = localMeters.x();
            m_device->position[1] = localMeters.y();

            emit deviceMovedMeters(QString::fromStdString(m_device->id),
                                   localMeters);
        }

        return QGraphicsEllipseItem::itemChange(change, value);
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent*) override {
        setPen(QPen(Qt::red, 2));
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override {
        setPen(QPen(Qt::darkYellow, 1));
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* e) override {
        if (e->button() == Qt::LeftButton && m_device) {
            emit deviceSelected("device",
                                QString::fromStdString(m_device->id));
            
            emit deviceSelectedDetailed(
                QString::fromStdString(m_device->id),
                QString::fromStdString(m_device->type),
                QPointF(m_device->position[0], m_device->position[1]),
                m_device->position[2]
            );
        }
        
        QGraphicsEllipseItem::mousePressEvent(e);
    }

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* e) override {
        QMenu menu;
        QAction* del = menu.addAction("Delete device");
        QAction* res = menu.exec(e->screenPos());
        if (res == del) {
            setSelected(true);
            if (scene()) {
                QMetaObject::invokeMethod(scene(), "deleteSelected",
                                          Qt::QueuedConnection);
            }
        }
    }

private:
    Room*   m_room   = nullptr;
    Device* m_device = nullptr;
    double  S;
    double  grid;
};

// ============================================================================
// ROUTE SEGMENT ITEM  (QObject + QGraphicsLineItem)
// ============================================================================

class RouteSegmentItem :
    public QObject,
    public QGraphicsLineItem
{
    Q_OBJECT

public:
    RouteSegmentItem(Room* room,
                     WireRoute* route,
                     RouteSegment* seg,
                     const QPointF& aPx,
                     const QPointF& bPx,
                     double scaleMetersToPx,
                     QGraphicsItem* parent = nullptr)
        : QObject(),
          QGraphicsLineItem(parent),
          m_room(room),
          m_route(route),
          m_segment(seg),
          S(scaleMetersToPx)
    {
        setLine(QLineF(aPx, bPx));
        setPen(QPen(routeColor(seg->type), 2));
        setFlags(ItemIsSelectable);
        setAcceptHoverEvents(true);
    }

    WireRoute* routeModel() const { return m_route; }
    RouteSegment* segmentModel() const { return m_segment; }

signals:
    void segmentSelected(QString type, QString id);
    void routeSelectedDetailed(QString id, QString type, QString room,
                               QPointF start, QPointF end,
                           double zStart, double zEnd);                               
    

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent*) override {
        QPen p = pen();
        p.setWidthF(3.0);
        setPen(p);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override {
        QPen p = pen();
        p.setWidthF(2.0);
        setPen(p);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* e) override {
        if (e->button() == Qt::LeftButton && m_route) {
            emit segmentSelected("route",
                                 QString::fromStdString(m_route->id));
        }
	emit segmentSelected("route", QString::fromStdString(m_route->id));

        // Retrieve the line geometry
        QLineF line = this->line();

        // Retrieve room origin (store this in addRoute)
        double ox = data(2).toDouble();
        double oy = data(3).toDouble();
        QPointF roomOriginPx(ox, oy);

        // Convert pixel → meter
        QPointF start_m = (line.p1() - roomOriginPx) / S;
        QPointF end_m   = (line.p2() - roomOriginPx) / S;

	emit routeSelectedDetailed(
	    QString::fromStdString(m_route->id),
	    QString::fromStdString(m_segment->type),
	    QString::fromStdString(m_segment->room_id),
	    start_m,
	    end_m,
        m_segment->start[2],
        m_segment->end[2]
	);
 
        QGraphicsLineItem::mousePressEvent(e);
    }

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* e) override {
        QMenu menu;
        QAction* del = menu.addAction("Delete segment");
        QAction* res = menu.exec(e->screenPos());
        if (res == del) {
            setSelected(true);
            if (scene()) {
                QMetaObject::invokeMethod(scene(), "deleteSelected",
                                          Qt::QueuedConnection);
            }
        }
    }

private:
    Room*         m_room   = nullptr;
    WireRoute*    m_route  = nullptr;
    RouteSegment* m_segment = nullptr;
    double        S;
};

// ============================================================================
// INSTALLATION SCENE
// ============================================================================

class InstallationScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit InstallationScene(QObject* parent = nullptr)
        : QGraphicsScene(parent) {}

    void setInstallation(Installation* inst) {
        installation = inst;
        rebuild();
    }

    void setLayers(const Layers& l) {
        layers = l;
        updateVisibility();
    }

    const Layers& layerState() const { return layers; }

signals:
    void deviceMoved(QString id, QPointF newPosMeters);
    void elementSelected(QString type, QString id);

    void deviceSelectedDetailed(QString id, QString type, QPointF posMeters, double zMeters);
    void routeSelectedDetailed(QString id, QString type, QString room,
                               QPointF start, QPointF end, double zStart, double zEnd);
    void wallSelectedDetailed(QString id, QPointF a, QPointF b, double thickness, double zStart, double zEnd);
    void roomSelectedDetailed(QString id, QPointF origin, QSizeF size, double z);

    void nothingSelected();

    

public slots:
    void deleteSelected() {
        for (auto* item : selectedItems()) {
            if (auto* dev = qgraphicsitem_cast<DeviceItem*>(item)) {
                removeDevice(dev);
            } else if (auto* seg = qgraphicsitem_cast<RouteSegmentItem*>(item)) {
                removeRouteSegment(seg);
            }
        }
    }

private:
    Installation* installation = nullptr;
    Layers layers;
    const double S = 80.0;
    const double gridPx = 20.0;

    // ---------------------- Build scene ----------------------

    void rebuild() {
        clear();
        if (!installation) return;

        for (auto& room : installation->rooms)
            addRoom(room);

        for (auto& route : installation->routes)
            addRoute(route);

        updateVisibility();
    }

    void updateVisibility() {
        for (auto* item : items()) {
            QVariant kind = item->data(0);
            if (!kind.isValid()) continue;

            QString k = kind.toString();
            bool vis = true;

            if (k == "wall")       vis = layers.walls;
            else if (k == "device") vis = layers.devices;
            else if (k == "route")  vis = layers.routes;
            else if (k == "roomLabel") vis = layers.roomLabels;

            item->setVisible(vis);
        }
    }

    // ---------------------- Rooms ----------------------

    void addRoom(Room& room) {
        QPointF origin(room.origin[0] * S, room.origin[1] * S);

        auto* rect = addRect(origin.x(),
                             origin.y(),
                             room.dimensions.width_m * S,
                             room.dimensions.length_m * S,
                             QPen(Qt::gray));
        rect->setZValue(-10);
        
        rect->setData(0, "room");
        rect->setData(1, QString::fromStdString(room.id));
        rect->setData(2, room.origin[2]);  // Floor Z position
        rect->setData(3, room.dimensions.height_m);  // Room height

        auto* label = addText(QString::fromStdString(room.id));
        label->setPos(origin + QPointF(5, 5));
        label->setData(0, "roomLabel");
        label->setZValue(10);

        for (auto& w : room.walls) {
            QPointF a = origin + QPointF(w.start[0] * S, w.start[1] * S);
            QPointF b = origin + QPointF(w.end[0] * S, w.end[1] * S);

            double thickness = w.thickness_m * S;
            auto* line = addLine(QLineF(a, b),
                                 QPen(Qt::black, thickness));
            line->setData(0, "wall");
            line->setData(1, QString::fromStdString(w.id));
            line->setData(2, origin.x());
            line->setData(3, origin.y());
            // Walls extend from floor (0) to ceiling (room height)
            line->setData(4, 0.0);  // Floor Z
            line->setData(5, room.dimensions.height_m);  // Ceiling Z
            line->setZValue(0);
        }

        for (auto& d : room.devices) {
            QPointF pos = origin + QPointF(d.position[0] * S, d.position[1] * S);
            auto* item = new DeviceItem(&room, &d, pos, S, gridPx);
            addItem(item);
            item->setData(0, "device");
            item->setData(1, QString::fromStdString(d.id));
            item->setZValue(20);

            connect(item, &DeviceItem::deviceMovedMeters,
                    this, &InstallationScene::deviceMoved);
            connect(item, &DeviceItem::deviceSelected,
                    this, &InstallationScene::elementSelected);        
            connect(item, &DeviceItem::deviceSelectedDetailed,
                    this, &InstallationScene::deviceSelectedDetailed);
        }

    }

    // ---------------------- Routes ----------------------

    void addRoute(WireRoute& route) {
        for (auto& seg : route.segments) {
            Room* room = nullptr;
            for (auto& r : installation->rooms)
                if (r.id == seg.room_id)
                    room = &r;

            if (!room) continue;

            QPointF origin(room->origin[0] * S, room->origin[1] * S);

            QPointF a = origin + QPointF(seg.start[0] * S, seg.start[1] * S);
            QPointF b = origin + QPointF(seg.end[0] * S, seg.end[1] * S);

            auto* item = new RouteSegmentItem(room, &route, &seg, a, b, S);
            addItem(item);
            item->setData(0, "route");
            item->setData(1, QString::fromStdString(route.id));
            item->setData(2, origin.x());
            item->setData(3, origin.y());
            item->setZValue(5);

            connect(item, &RouteSegmentItem::segmentSelected,
                    this, &InstallationScene::elementSelected);
            connect(item, &RouteSegmentItem::routeSelectedDetailed,
                    this, &InstallationScene::routeSelectedDetailed);
        }
        


        
    }

    // ---------------------- Deletion helpers ----------------------

    void removeDevice(DeviceItem* dev) {
        auto* room = dev->room();
        auto* model = dev->deviceModel();
        if (!room || !model) { removeItem(dev); delete dev; return; }

        auto& vec = room->devices;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
                                 [&](const Device& d){ return &d == model; }),
                  vec.end());
        removeItem(dev);
        delete dev;
    }

    void removeRouteSegment(RouteSegmentItem* segItem) {
        auto* route = segItem->routeModel();
        auto* seg   = segItem->segmentModel();
        if (!route || !seg) { removeItem(segItem); delete segItem; return; }

        auto& vec = route->segments;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
                                 [&](const RouteSegment& s){ return &s == seg; }),
                  vec.end());
        removeItem(segItem);
        delete segItem;
    }
    
void mousePressEvent(QGraphicsSceneMouseEvent* e) override {
    QGraphicsItem* item = itemAt(e->scenePos(), QTransform());

    if (!item) {
        emit nothingSelected();
        QGraphicsScene::mousePressEvent(e);
        return;
    }

    QVariant kind = item->data(0);
    if (!kind.isValid()) {
        emit nothingSelected();
        QGraphicsScene::mousePressEvent(e);
        return;
    }

    QString k = kind.toString();

        // ---------------- Room selection ----------------
if (k == "room") {
    QString id = item->data(1).toString();
    auto* rectItem = static_cast<QGraphicsRectItem*>(item);
    QRectF r = rectItem->rect();

    // Convert pixel → meter
    QPointF origin_m(r.x() / S, r.y() / S);
    QSizeF size_m(r.width() / S, r.height() / S);

    // Retrieve room height from dimensions
    double height = item->data(3).toDouble();
    
    emit roomSelectedDetailed(id, origin_m, size_m, height);
}


        // ---------------- Wall selection ----------------
if (k == "wall") {
    QString id = item->data(1).toString();
    auto* lineItem = static_cast<QGraphicsLineItem*>(item);

    // Retrieve the line geometry
    QLineF line = lineItem->line();

    // Retrieve room origin (stored earlier)
    double ox = item->data(2).toDouble();
    double oy = item->data(3).toDouble();
    QPointF roomOriginPx(ox, oy);

    // Convert pixel → meter
    QPointF start_m = (line.p1() - roomOriginPx) / S;
    QPointF end_m   = (line.p2() - roomOriginPx) / S;

    // Convert thickness
    double thickness_px = lineItem->pen().widthF();
    double thickness_m = thickness_px / S;

    // Retrieve Z
    double zStart = item->data(4).toDouble();
    double zEnd   = item->data(5).toDouble();

    emit wallSelectedDetailed(id, start_m, end_m, thickness_m, zStart, zEnd);
}




    QGraphicsScene::mousePressEvent(e);
}
    
    
};

