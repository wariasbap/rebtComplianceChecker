#pragma once
#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QStackedWidget>

class InspectorPanel : public QWidget {
    Q_OBJECT

public:
    explicit InspectorPanel(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* layout = new QVBoxLayout(this);

        title = new QLabel("<b>No selection</b>");
        layout->addWidget(title);

        stack = new QStackedWidget();
        layout->addWidget(stack);

        // ---------------- Device panel ----------------
        auto* deviceBox = new QWidget();
        auto* deviceForm = new QFormLayout(deviceBox);

        devId = new QLabel("-");
        devType = new QLabel("-");
        devX = new QDoubleSpinBox();
        devY = new QDoubleSpinBox();
        devZ = new QDoubleSpinBox();
        devX->setRange(-9999, 9999);
        devY->setRange(-9999, 9999);
        devZ->setRange(-9999, 9999);

        deviceForm->addRow("ID:", devId);
        deviceForm->addRow("Type:", devType);
        deviceForm->addRow("X (m):", devX);
        deviceForm->addRow("Y (m):", devY);
        deviceForm->addRow("Z (m):", devZ);

        stack->addWidget(deviceBox);

        // ---------------- Route segment panel ----------------
        auto* routeBox = new QWidget();
        auto* routeForm = new QFormLayout(routeBox);

        routeId = new QLabel("-");
        routeType = new QLabel("-");
        routeRoom = new QLabel("-");
        routeStart = new QLabel("-");
        routeEnd = new QLabel("-");
        routeZStart = new QDoubleSpinBox();
        routeZEnd = new QDoubleSpinBox();
        routeZStart->setRange(-9999, 9999);
        routeZEnd->setRange(-9999, 9999);

        routeForm->addRow("Route ID:", routeId);
        routeForm->addRow("Type:", routeType);
        routeForm->addRow("Room:", routeRoom);
        routeForm->addRow("Start:", routeStart);
        routeForm->addRow("End:", routeEnd);
        routeForm->addRow("Z Start (m):", routeZStart);
        routeForm->addRow("Z End (m):", routeZEnd);

        stack->addWidget(routeBox);

        // ---------------- Wall panel ----------------
        auto* wallBox = new QWidget();
        auto* wallForm = new QFormLayout(wallBox);

        wallId = new QLabel("-");
        wallStart = new QLabel("-");
        wallEnd = new QLabel("-");
        wallThickness = new QLabel("-");
        wallZStart = new QDoubleSpinBox();
        wallZEnd = new QDoubleSpinBox();
        wallZStart->setRange(-9999, 9999);
        wallZEnd->setRange(-9999, 9999);

        wallForm->addRow("Wall ID:", wallId);
        wallForm->addRow("Start:", wallStart);
        wallForm->addRow("End:", wallEnd);
        wallForm->addRow("Thickness:", wallThickness);
        wallForm->addRow("Z Start (m):", wallZStart);
        wallForm->addRow("Z End (m):", wallZEnd);

        stack->addWidget(wallBox);

        // ---------------- Room panel ----------------
        auto* roomBox = new QWidget();
        auto* roomForm = new QFormLayout(roomBox);

        roomId = new QLabel("-");
        roomOrigin = new QLabel("-");
        roomSize = new QLabel("-");
        roomZ = new QDoubleSpinBox();
        roomZ->setRange(-9999, 9999);

        roomForm->addRow("Room ID:", roomId);
        roomForm->addRow("Origin:", roomOrigin);
        roomForm->addRow("Size:", roomSize);
        roomForm->addRow("Height (m):", roomZ);

        stack->addWidget(roomBox);

        stack->setCurrentIndex(0);
    }

public slots:
    void showDevice(QString id, QString type, QPointF posMeters, double zMeters) {
        title->setText("<b>Device</b>");
        devId->setText(id);
        devType->setText(type);
        devX->setValue(posMeters.x());
        devY->setValue(posMeters.y());
        devZ->setValue(zMeters);
        stack->setCurrentIndex(0);

    }

    void showRoute(QString id, QString type,
                   QString room,
                   QPointF start, QPointF end,
                               double zStart, double zEnd) {
        title->setText("<b>Route Segment</b>");
        routeId->setText(id);
        routeType->setText(type);
        routeRoom->setText(room);
        routeStart->setText(QString("(%1, %2)").arg(start.x()).arg(start.y()));
        routeEnd->setText(QString("(%1, %2)").arg(end.x()).arg(end.y()));
        stack->setCurrentIndex(1);
        routeZStart->setValue(zStart);
        routeZEnd->setValue(zEnd);
    }

    void showWall(QString id, QPointF a, QPointF b, double thick, double zStart, double zEnd) {
        title->setText("<b>Wall</b>");
        wallId->setText(id);
        wallStart->setText(QString("(%1, %2)").arg(a.x()).arg(a.y()));
        wallEnd->setText(QString("(%1, %2)").arg(b.x()).arg(b.y()));
        wallThickness->setText(QString::number(thick));
        stack->setCurrentIndex(2);
        wallZStart->setValue(zStart);
        wallZEnd->setValue(zEnd);
    }

    void showRoom(QString id, QPointF origin, QSizeF size, double z) {
        title->setText("<b>Room</b>");
        roomId->setText(id);
        roomOrigin->setText(QString("(%1, %2)").arg(origin.x()).arg(origin.y()));
        roomSize->setText(QString("%1 × %2 m").arg(size.width()).arg(size.height()));
        stack->setCurrentIndex(3);
        roomZ->setValue(z);
    }

    void clearPanel() {
        title->setText("<b>No selection</b>");
        stack->setCurrentIndex(0);
    }

private:
    QLabel* title;

    QStackedWidget* stack;

    // Device
    QLabel* devId;
    QLabel* devType;
    QDoubleSpinBox* devX;
    QDoubleSpinBox* devY;
    QDoubleSpinBox* devZ;


    // Route
    QLabel* routeId;
    QLabel* routeType;
    QLabel* routeRoom;
    QLabel* routeStart;
    QLabel* routeEnd;
    QDoubleSpinBox* routeZStart;
    QDoubleSpinBox* routeZEnd;

    // Wall
    QLabel* wallId;
    QLabel* wallStart;
    QLabel* wallEnd;
    QLabel* wallThickness;
    QDoubleSpinBox* wallZStart;
    QDoubleSpinBox* wallZEnd;

    // Room
    QLabel* roomId;
    QLabel* roomOrigin;
    QLabel* roomSize;
    QDoubleSpinBox* roomZ;
};
