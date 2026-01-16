// main.cpp

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "JsonAdapters.hpp"
#include "Visualise.hpp"
#include <QApplication>
#include <QMainWindow>
#include "InstallationView.hpp"

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

    std::cout << "Installation: " << inst.name
              << " (standard: " << inst.standard << ")\n";
    std::cout << "Rooms: " << inst.rooms.size()
              << ", Circuits: " << inst.circuits.size()
              << ", Routes: " << inst.routes.size() << "\n";

    std::string svg = visualiseInstallation(inst);
    std::ofstream out("installation.svg");
    out << svg;

    auto* view = new InstallationView();
    view->setInstallation(&inst);

    QMainWindow win;
    win.setCentralWidget(view);
    win.resize(1200, 800);
    win.show(); 

    return app.exec();

    //return 0;
}

