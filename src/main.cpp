#include <QApplication>
#include <QMainWindow>
#include <QDockWidget>
#include <iostream>
#include <fstream>

#include "InstallationScene.hpp"
#include "InstallationView.hpp"
#include "InspectorPanel.hpp"
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

    Installation inst = j.get<Installation>();

    auto* scene = new InstallationScene();
    scene->setInstallation(&inst);

    auto* view = new InstallationView();
    view->setScene(scene);

    QObject::connect(scene, &InstallationScene::elementSelected,
                     [](QString type, QString id){
                         qDebug() << "Selected:" << type << id;
                     });

    QMainWindow win;
    win.setCentralWidget(view);
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

