#include <QCoreApplication>
#include "wavmanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString wavFolder = "resources/waves/";

    WavManager manager(wavFolder);
    if (manager.loadWavFiles()) {
        qDebug() << "Read files:" << manager.availableWavs();

        // Test playing sound
        if (manager.playWav("themes.wav")) {
            QTimer::singleShot(2000, &a, &QCoreApplication::quit); // wait 2 seconds
            return a.exec();
        } else {
            qCritical() << "Could not play the requested file";
        }
    } else {
        qCritical() << "Couldn't load the requested wave file";
    }
    return 0;
}
