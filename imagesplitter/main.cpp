#include <QCoreApplication>
#include "ImageSplitter.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Ställ in sökvägen till din bildfil här
    QString imagePath = "mordor_image_1.bin";

    ImageSplitter splitter;

    QObject::connect(&splitter, &ImageSplitter::errorOccurred,
                     [](const QString &msg){
        qCritical() << "ERROR:" << msg;
    });

    QList<QImage> splitImages = splitter.splitImage(imagePath);

    if (!splitImages.isEmpty()) {
        qDebug() << "Total pieces created:" << splitImages.size();

        // Exempel på hur man sparar de delade bilderna
        for (int i = 0; i < splitImages.size(); ++i) {
            QString savePath = QString("piece_%1.png").arg(i + 1);
            if (splitImages.at(i).save(savePath)) {
                qDebug() << "Saved piece" << i + 1 << "to" << savePath;
            } else {
                qWarning() << "Failed to save piece" << i + 1;
            }
        }
    } else {
        qDebug() << "Image splitting failed or resulted in no pieces.";
    }

    // Observera: Denna QCoreApplication kommer att avslutas omedelbart
    // efter att arbetet är klart, eftersom den inte har någon event-loop.
    // I ett GUI-program (QApplication) skulle den köras tills fönstret stängs.
    return 0;
}
