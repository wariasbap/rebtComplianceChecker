#pragma once
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QGeometryRenderer>
#include <QWidget>

#include "Geometry.hpp"
#include "Electrical.hpp"
#include "Routing.hpp"

class Installation3DView : public QWidget {
    Q_OBJECT

public:
    explicit Installation3DView(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        // Create the 3D window
        view3D = new Qt3DExtras::Qt3DWindow();
        view3D->defaultFrameGraph()->setClearColor(QColor(240, 240, 240));
        
        // Create container widget
        container = QWidget::createWindowContainer(view3D, this);
        
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(container);
        
        // Root entity
        rootEntity = new Qt3DCore::QEntity();
        
        // Camera setup
        setupCamera();
        
        // Lighting
        setupLighting();
        
        view3D->setRootEntity(rootEntity);
    }
    
    void setInstallation(Installation* inst) {
        installation = inst;
        buildScene();
    }

private:
    Qt3DExtras::Qt3DWindow* view3D;
    QWidget* container;
    Qt3DCore::QEntity* rootEntity;
    Installation* installation = nullptr;
    
    // Create a wireframe material for walls
    Qt3DExtras::QPhongMaterial* createWireframeMaterial() {
        auto* material = new Qt3DExtras::QPhongMaterial();
        material->setDiffuse(QColor(100, 100, 120));
        material->setAmbient(QColor(80, 80, 100));
        material->setShininess(0.0f);
        return material;
    }
    
    void setupCamera() {
        Qt3DRender::QCamera* camera = view3D->camera();
        camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
        camera->setPosition(QVector3D(10, 10, 20));
        camera->setViewCenter(QVector3D(0, 0, 0));
        
        // Camera controller for mouse interaction
        auto* camController = new Qt3DExtras::QOrbitCameraController(rootEntity);
        camController->setCamera(camera);
    }
    
    void setupLighting() {
        // Add a point light
        auto* lightEntity = new Qt3DCore::QEntity(rootEntity);
        auto* light = new Qt3DRender::QPointLight(lightEntity);
        light->setColor(Qt::white);
        light->setIntensity(1.0f);
        
        auto* lightTransform = new Qt3DCore::QTransform(lightEntity);
        lightTransform->setTranslation(QVector3D(10, 10, 10));
        
        lightEntity->addComponent(light);
        lightEntity->addComponent(lightTransform);
    }
    
    void buildScene() {
        if (!installation) return;
        
        // Clear existing scene (except camera and lights)
        // For simplicity, we'll just rebuild everything
        
        for (auto& room : installation->rooms) {
            addRoom(room);
        }
        
        for (auto& route : installation->routes) {
            addRoute(route);
        }
    }
    
    void addRoom(const Room& room) {
        // Room origin in world space
        QVector3D origin(room.origin[0], room.origin[2], -room.origin[1]); // Y-up convention
        
        // Draw floor as a thin cuboid (not a plane) to avoid rendering artifacts
        auto* floorEntity = new Qt3DCore::QEntity(rootEntity);
        auto* floorMesh = new Qt3DExtras::QCuboidMesh();
        floorMesh->setXExtent(room.dimensions.width_m);
        floorMesh->setYExtent(0.05f);  // Thin floor slab
        floorMesh->setZExtent(room.dimensions.length_m);
        
        auto* floorMaterial = new Qt3DExtras::QPhongMaterial();
        floorMaterial->setDiffuse(QColor(200, 200, 200));
        floorMaterial->setAmbient(QColor(150, 150, 150));
        
        auto* floorTransform = new Qt3DCore::QTransform();
        // Position floor center at room origin, offset by half dimensions
        floorTransform->setTranslation(origin + QVector3D(room.dimensions.width_m/2, -0.025f, -room.dimensions.length_m/2));
        
        floorEntity->addComponent(floorMesh);
        floorEntity->addComponent(floorMaterial);
        floorEntity->addComponent(floorTransform);
        
        // Draw walls
        for (const auto& wall : room.walls) {
            addWall(wall, room, origin);
        }
        
        // Draw devices
        for (const auto& device : room.devices) {
            addDevice(device, room, origin);
        }
    }
    
    void addWall(const Wall& wall, const Room& room, const QVector3D& roomOrigin) {
        // Wall start and end in room coordinates
        QVector3D start(wall.start[0], wall.start[2], -wall.start[1]);
        QVector3D end(wall.end[0], wall.end[2], -wall.end[1]);
        
        // Calculate wall dimensions
        QVector3D wallDir = end - start;
        float wallHeight = room.dimensions.height_m;
        float wallThickness = wall.thickness_m;
        
        // Create wireframe by drawing the 12 edges of the cuboid
        // Instead of a solid wall, we'll draw thin cylinders for each edge
        
        wallDir.normalize();
        QVector3D upVector(0, 1, 0);
        QVector3D rightVector = QVector3D::crossProduct(wallDir, upVector);
        rightVector.normalize();
        
        // 8 corners of the wall box
        QVector3D p0 = roomOrigin + start - rightVector * wallThickness/2;  // bottom-left-front
        QVector3D p1 = roomOrigin + end - rightVector * wallThickness/2;    // bottom-right-front
        QVector3D p2 = roomOrigin + start + rightVector * wallThickness/2;  // bottom-left-back
        QVector3D p3 = roomOrigin + end + rightVector * wallThickness/2;    // bottom-right-back
        QVector3D p4 = p0 + upVector * wallHeight;  // top-left-front
        QVector3D p5 = p1 + upVector * wallHeight;  // top-right-front
        QVector3D p6 = p2 + upVector * wallHeight;  // top-left-back
        QVector3D p7 = p3 + upVector * wallHeight;  // top-right-back
        
        // Draw the 12 edges as thin cylinders
        QColor wireColor(80, 80, 100);
        float wireThickness = 0.02f;
        
        // Bottom face edges
        addWireEdge(p0, p1, wireColor, wireThickness);
        addWireEdge(p1, p3, wireColor, wireThickness);
        addWireEdge(p3, p2, wireColor, wireThickness);
        addWireEdge(p2, p0, wireColor, wireThickness);
        
        // Top face edges
        addWireEdge(p4, p5, wireColor, wireThickness);
        addWireEdge(p5, p7, wireColor, wireThickness);
        addWireEdge(p7, p6, wireColor, wireThickness);
        addWireEdge(p6, p4, wireColor, wireThickness);
        
        // Vertical edges
        addWireEdge(p0, p4, wireColor, wireThickness);
        addWireEdge(p1, p5, wireColor, wireThickness);
        addWireEdge(p2, p6, wireColor, wireThickness);
        addWireEdge(p3, p7, wireColor, wireThickness);
    }
    
    // Helper function to add a wire edge (thin cylinder)
    void addWireEdge(const QVector3D& start, const QVector3D& end, 
                     const QColor& color, float thickness) {
        QVector3D dir = end - start;
        float length = dir.length();
        
        if (length < 0.001f) return;
        
        auto* edgeEntity = new Qt3DCore::QEntity(rootEntity);
        auto* edgeMesh = new Qt3DExtras::QCuboidMesh();
        edgeMesh->setXExtent(thickness);
        edgeMesh->setYExtent(length);
        edgeMesh->setZExtent(thickness);
        
        auto* edgeMaterial = new Qt3DExtras::QPhongMaterial();
        edgeMaterial->setDiffuse(color);
        edgeMaterial->setAmbient(color.darker());
        
        auto* edgeTransform = new Qt3DCore::QTransform();
        edgeTransform->setTranslation((start + end) / 2.0f);
        
        // Rotate to align with direction
        dir.normalize();
        QVector3D yAxis(0, 1, 0);
        float dotProduct = QVector3D::dotProduct(yAxis, dir);
        
        if (std::abs(dotProduct) < 0.999f) {
            QVector3D axis = QVector3D::crossProduct(yAxis, dir);
            if (axis.length() > 0.001f) {
                float angle = qRadiansToDegrees(std::acos(dotProduct));
                edgeTransform->setRotation(QQuaternion::fromAxisAndAngle(axis, angle));
            }
        } else if (dotProduct < 0) {
            edgeTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), 180));
        }
        
        edgeEntity->addComponent(edgeMesh);
        edgeEntity->addComponent(edgeMaterial);
        edgeEntity->addComponent(edgeTransform);
    }
    
    void addDevice(const Device& device, const Room& /*room*/, const QVector3D& roomOrigin) {
        auto* deviceEntity = new Qt3DCore::QEntity(rootEntity);
        auto* deviceMesh = new Qt3DExtras::QSphereMesh();
        deviceMesh->setRadius(0.10f);  // Smaller radius so routes are more visible
        
        auto* deviceMaterial = new Qt3DExtras::QPhongMaterial();
        
        // Color by device type
        if (device.type == "light") {
            deviceMaterial->setDiffuse(QColor(255, 255, 100));
        } else if (device.type == "socket") {
            deviceMaterial->setDiffuse(QColor(100, 100, 255));
        } else if (device.type == "switch") {
            deviceMaterial->setDiffuse(QColor(255, 100, 100));
        } else {
            deviceMaterial->setDiffuse(QColor(150, 150, 150));
        }
        
        auto* deviceTransform = new Qt3DCore::QTransform();
        QVector3D pos = roomOrigin + QVector3D(device.position[0], device.position[2], -device.position[1]);
        deviceTransform->setTranslation(pos);
        
        deviceEntity->addComponent(deviceMesh);
        deviceEntity->addComponent(deviceMaterial);
        deviceEntity->addComponent(deviceTransform);
    }
    
    void addRoute(const WireRoute& route) {
        // For now, we'll draw routes as simple lines between points
        // In a more sophisticated version, you'd use cylinder meshes
        for (const auto& segment : route.segments) {
            // Find the room for this segment
            const Room* room = nullptr;
            for (const auto& r : installation->rooms) {
                if (r.id == segment.room_id) {
                    room = &r;
                    break;
                }
            }
            
            if (!room) continue;
            
            QVector3D roomOrigin(room->origin[0], room->origin[2], -room->origin[1]);
            QVector3D start = roomOrigin + QVector3D(segment.start[0], segment.start[2], -segment.start[1]);
            QVector3D end = roomOrigin + QVector3D(segment.end[0], segment.end[2], -segment.end[1]);
            
            // Draw as a thin cylinder
            QVector3D dir = end - start;
            float length = dir.length();
            
            if (length < 0.001f) continue;
            
            // Debug output for vertical drops
            if (segment.type == "vertical_drop") {
                qDebug() << "Vertical drop:" 
                         << "start:" << start.x() << start.y() << start.z()
                         << "end:" << end.x() << end.y() << end.z()
                         << "length:" << length;
            }
            
            auto* routeEntity = new Qt3DCore::QEntity(rootEntity);
            auto* routeMesh = new Qt3DExtras::QCuboidMesh();
            
            // Make vertical drops much thicker and more visible (they're usually short)
            float thickness = (segment.type == "vertical_drop") ? 0.15f : 0.05f;
            routeMesh->setXExtent(thickness);
            routeMesh->setYExtent(length);
            routeMesh->setZExtent(thickness);
            
            auto* routeMaterial = new Qt3DExtras::QPhongMaterial();
            
            // Color by route type (matching 2D view colors)
            if (segment.type == "ceiling") routeMaterial->setDiffuse(QColor("#00CC44"));
            else if (segment.type == "wall_chase") routeMaterial->setDiffuse(QColor("#FF8800"));
            else if (segment.type == "vertical_drop") {
                routeMaterial->setDiffuse(QColor("#FF0000"));
                routeMaterial->setAmbient(QColor("#AA0000")); // Make it brighter
                routeMaterial->setShininess(100.0f);
            }
            else if (segment.type == "conduit") routeMaterial->setDiffuse(QColor("#0066FF"));
            else if (segment.type == "free_air") routeMaterial->setDiffuse(QColor("#AA00FF"));
            else routeMaterial->setDiffuse(QColor("#000000"));
            
            auto* routeTransform = new Qt3DCore::QTransform();
            routeTransform->setTranslation((start + end) / 2.0f);
            
            // Rotate to align with direction
            // The cuboid's Y extent is along the Y axis by default
            dir.normalize();
            QVector3D yAxis(0, 1, 0);
            
            // Check if dir is already aligned with Y axis (vertical segment)
            float dotProduct = QVector3D::dotProduct(yAxis, dir);
            if (std::abs(dotProduct) < 0.999f) {  // Not aligned with Y axis
                QVector3D axis = QVector3D::crossProduct(yAxis, dir);
                if (axis.length() > 0.001f) {
                    float angle = qRadiansToDegrees(std::acos(dotProduct));
                    routeTransform->setRotation(QQuaternion::fromAxisAndAngle(axis, angle));
                }
            } else if (dotProduct < 0) {
                // Pointing down, rotate 180 degrees
                routeTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), 180));
            }
            // If dotProduct > 0.999, it's already aligned upward, no rotation needed
            
            routeEntity->addComponent(routeMesh);
            routeEntity->addComponent(routeMaterial);
            routeEntity->addComponent(routeTransform);
        }
    }
};
