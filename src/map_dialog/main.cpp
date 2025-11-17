#include <QApplication>
#include "MapDialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MapDialog w;
    w.show();
    return a.exec();
}
