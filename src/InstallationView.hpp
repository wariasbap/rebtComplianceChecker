#pragma once
#include <QGraphicsView>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>

#include "InstallationScene.hpp"

class InstallationView : public QGraphicsView {
    Q_OBJECT
public:
    explicit InstallationView(QWidget* parent = nullptr)
        : QGraphicsView(parent) {
        setRenderHint(QPainter::Antialiasing);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    }

    void setSceneWithLayers(InstallationScene* s) {
        m_scene = s;
        setScene(s);
    }

protected:
    void wheelEvent(QWheelEvent* e) override {
        double factor = (e->angleDelta().y() > 0) ? 1.15 : 0.85;
        scale(factor, factor);
    }

    void keyPressEvent(QKeyEvent* e) override {
        if (!m_scene) { QGraphicsView::keyPressEvent(e); return; }

        Layers layers = m_scene->layerState();
        bool handled = true;

        switch (e->key()) {
        case Qt::Key_W: layers.walls      = !layers.walls; break;
        case Qt::Key_D: layers.devices    = !layers.devices; break;
        case Qt::Key_R: layers.routes     = !layers.routes; break;
        case Qt::Key_L: layers.roomLabels = !layers.roomLabels; break;
        case Qt::Key_Delete:
            m_scene->deleteSelected();
            break;
        default:
            handled = false;
            break;
        }

        if (handled && e->key() != Qt::Key_Delete) {
            m_scene->setLayers(layers);
        }

        if (!handled)
            QGraphicsView::keyPressEvent(e);
    }

    void drawBackground(QPainter* painter, const QRectF& rect) override {
        if (!m_scene) {
            QGraphicsView::drawBackground(painter, rect);
            return;
        }

        if (!m_scene->layerState().grid) {
            QGraphicsView::drawBackground(painter, rect);
            return;
        }

        painter->fillRect(rect, Qt::white);

        const double grid = 20.0;
        QPen pen(QColor(230, 230, 230));
        painter->setPen(pen);

        double left   = std::floor(rect.left() / grid) * grid;
        double right  = std::ceil(rect.right() / grid) * grid;
        double top    = std::floor(rect.top() / grid) * grid;
        double bottom = std::ceil(rect.bottom() / grid) * grid;

        for (double x = left; x <= right; x += grid)
            painter->drawLine(QLineF(x, top, x, bottom));
        for (double y = top; y <= bottom; y += grid)
            painter->drawLine(QLineF(left, y, right, y));
    }

private:
    InstallationScene* m_scene = nullptr;
};

