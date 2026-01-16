#include <QApplication>
#include <QMainWindow>
#include <iostream>
#include <fstream>

#include "InstallationScene.hpp"
#include "InstallationView.hpp"
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

    QMainWindow win;
    win.setCentralWidget(view);
    win.resize(1400, 900);
    win.show();

    return app.exec();
}

