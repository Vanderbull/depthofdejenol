#include <QApplication>
#include "csvviewer.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    CSVViewer viewer;
    viewer.show();
    return app.exec();
}
