#include "complicationeffecteditor.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    ComplicationEffectEditor w;
    w.show();
    return app.exec();
}
