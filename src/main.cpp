#include <QApplication>
#include <QMainWindow>
#include <QDockWidget>
#include <QTabWidget>
#include <iostream>
#include <fstream>

#include "InstallationScene.hpp"
#include "InstallationView.hpp"
#include "InspectorPanel.hpp"
#include "Installation3DView.hpp"
#include "JsonAdapters.hpp"

int main(int argc, char** argv) {
    QApplication app(argc, argv);


    if (argc < 2) {
        std::cerr << "Usage: rebt_checker installation.json\n";
        return 1;
    }

    std::ifstream f(argv[1]);
    if (!f) {
        std::cerr << "Cannot open file: " << argv[1] << "\n";
        return 1;
    }

    nlohmann::json j;
    f >> j;

    auto* inst = new Installation(j.get<Installation>());

    auto* scene = new InstallationScene();
    scene->setInstallation(inst);

    auto* view = new InstallationView();
    view->setScene(scene);
    
    // Create 3D view
    auto* view3D = new Installation3DView();
    view3D->setInstallation(inst);
    
    // Create tab widget to switch between 2D and 3D views
    auto* tabWidget = new QTabWidget();
    tabWidget->addTab(view, "2D View");
    tabWidget->addTab(view3D, "3D View");

    QObject::connect(scene, &InstallationScene::elementSelected,
                     [](QString type, QString id){
                         qDebug() << "Selected:" << type << id;
                     });

    QMainWindow win;
    win.setCentralWidget(tabWidget);
    win.resize(1400, 900);
    win.show();

auto* inspector = new InspectorPanel();
auto* dock = new QDockWidget("Inspector");
dock->setWidget(inspector);
win.addDockWidget(Qt::RightDockWidgetArea, dock);

// Connect scene → inspector
QObject::connect(scene, &InstallationScene::deviceSelectedDetailed,
                 inspector, &InspectorPanel::showDevice);

QObject::connect(scene, &InstallationScene::routeSelectedDetailed,
                 inspector, &InspectorPanel::showRoute);

QObject::connect(scene, &InstallationScene::wallSelectedDetailed,
                 inspector, &InspectorPanel::showWall);

QObject::connect(scene, &InstallationScene::roomSelectedDetailed,
                 inspector, &InspectorPanel::showRoom);

    return app.exec();
}

