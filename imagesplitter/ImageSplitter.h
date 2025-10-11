#ifndef IMAGESPLITTER_H
#define IMAGESPLITTER_H

#include <QObject>
#include <QImage>
#include <QList>

class ImageSplitter : public QObject
{
    Q_OBJECT
public:
    explicit ImageSplitter(QObject *parent = nullptr);

    /**
     * @brief Loads an image and splits it into a grid of 2 rows and 24 columns (48 pieces total).
     * @param filePath The path to the image file.
     * @return A QList of QImage objects representing the split pieces.
     * Returns an empty list if the image fails to load or has invalid dimensions.
     */
    QList<QImage> splitImage(const QString &filePath);

private:
    const int ROWS = 2;       // Number of rows (image height / 2)
    const int COLS = 25;      // Number of columns

signals:
    // Optional: Signal to report errors
    void errorOccurred(const QString &message);
};

#endif // IMAGESPLITTER_H
