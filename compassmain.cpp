#include <QApplication>
#include "CompassWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CompassWidget w;
    w.resize(300, 300);
    w.setWindowTitle("Compass Demo");
    w.show();

    return a.exec();
}
