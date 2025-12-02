#include <QCoreApplication>
#include "wavmanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 1. ANPASSA DENNA SÖKVÄG:
    // Ersätt med den faktiska mappen där dina WAV-filer ligger.
    // Exempel: "/home/användare/ljud/wavs" eller "C:/ljud/wavs"
    QString wavFolder = "sökväg/till/din/wav/mapp"; // <-- Ändra detta!

    WavManager manager(wavFolder);

    // 2. Ladda filerna
    if (manager.loadWavFiles()) {
        qDebug() << "Inlästa ljud:" << manager.availableWavs();

        // 3. Testa att spela upp en fil
        if (manager.playWav("filnamn1")) { // <-- Ersätt "filnamn1" med ett av dina riktiga filnamn (utan .wav)
            // Ljudet spelas. Eftersom det är QCoreApplication (utan eventloop)
            // och QSoundEffect spelas asynkront, behöver vi låta programmet köra ett tag.
            // I ett GUI-program (med QApplication) skulle detta inte behöva göras.

            // Här är en liten "hack" för att vänta, använd inte detta i produktion.
            // I ett riktigt program hanteras detta av huvudapplikationens event-loop.
            QTimer::singleShot(2000, &a, &QCoreApplication::quit); // Vänta 2 sekunder
            return a.exec();
        } else {
            qCritical() << "Kunde inte spela upp den önskade filen.";
        }
    } else {
        qCritical() << "Kunde inte ladda WAV-filer från mappen.";
    }

    return 0;
}
