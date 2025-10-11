#include "ImageSplitter.h"
#include <QDebug>

ImageSplitter::ImageSplitter(QObject *parent)
    : QObject{parent}
{
}

QList<QImage> ImageSplitter::splitImage(const QString &filePath)
{
    QList<QImage> pieces;
    QImage originalImage;

    if (!originalImage.load(filePath)) {
        emit errorOccurred(tr("Failed to load image from path: %1").arg(filePath));
        return pieces; // Return empty list on failure
    }

    int totalWidth = originalImage.width();
    int totalHeight = originalImage.height();

    // Check if the image dimensions are suitable for the requested split
    if (totalWidth < COLS || totalHeight < ROWS) {
        emit errorOccurred(tr("Image dimensions (%1x%2) are too small for a %3x%4 split.")
                           .arg(totalWidth).arg(totalHeight).arg(COLS).arg(ROWS));
        return pieces;
    }

    // Calculate the dimensions of each piece
    // Note: Integer division is used. Any remainder pixels will be handled by the last row/column pieces.
    int pieceWidth = totalWidth / COLS;
    int pieceHeight = totalHeight / ROWS;

    // Use loop to iterate through the rows and columns
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            // Calculate the starting coordinates (x, y) for the current piece
            int x = col * pieceWidth;
            int y = row * pieceHeight;

            // Calculate the actual width and height for the current piece.
            // This handles the remainder pixels for the last row and column to ensure all pixels are included.
            int currentPieceWidth = (col == COLS - 1) ? (totalWidth - x) : pieceWidth;
            int currentPieceHeight = (row == ROWS - 1) ? (totalHeight - y) : pieceHeight;

            if (currentPieceWidth > 0 && currentPieceHeight > 0) {
                // Extract the sub-image using QImage::copy(x, y, w, h)
                QImage piece = originalImage.copy(x, y, currentPieceWidth, currentPieceHeight);
                pieces.append(piece);
            } else {
                qWarning() << "Skipped splitting piece with zero dimension at row:" << row << "col:" << col;
            }
        }
    }

    qDebug() << "Image successfully split into" << pieces.size() << "pieces.";
    return pieces;
}
