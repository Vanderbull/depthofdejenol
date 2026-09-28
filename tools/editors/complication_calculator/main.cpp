#include "complicationcalculatoreditor.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    ComplicationCalcEditor w;
    w.show();
    return app.exec();
}
