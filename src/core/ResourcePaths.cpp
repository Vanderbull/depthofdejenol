#include "ResourcePaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

QString ResourcePaths::locate(const QString &fileName)
{
    if (fileName.isEmpty())
        return QString();

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString cwd = QDir::currentPath();

    QStringList candidates;
    candidates << appDir
               << appDir + "/.."
               << appDir + "/../.."
               << appDir + "/../../src"
               << cwd
               << cwd + "/src";

    // Last resort: the directory this file was compiled from. Only valid on the
    // machine that built the binary, but it keeps an uninstalled dev build
    // working no matter which directory it is launched from.
    candidates << QFileInfo(QString::fromUtf8(__FILE__)).absolutePath();

    for (const QString &dir : candidates) {
        const QString path = QDir(dir).filePath(fileName);
        const QFileInfo info(path);
        if (info.exists() && info.isFile())
            return info.absoluteFilePath();
    }

    return QString();
}
