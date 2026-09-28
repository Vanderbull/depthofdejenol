#include "gamedataeditoreditor.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    GameDataEditor w;
    w.show();
    return app.exec();
}
