#pragma once
#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>

class InstallationView : public QGraphicsView {
    Q_OBJECT

public:
    explicit InstallationView(QWidget* parent = nullptr)
        : QGraphicsView(parent) {
        setRenderHint(QPainter::Antialiasing);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    }

protected:
    void wheelEvent(QWheelEvent* e) override {
        double factor = (e->angleDelta().y() > 0) ? 1.15 : 0.85;
        scale(factor, factor);
    }
};

