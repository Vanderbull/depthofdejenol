#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDateTime>
#include <algorithm>
#include <QPainter>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QPixmap>
#include <QSize>
#include <QList>
#include <QPair>
#include <QObject>
#include <QWidget>

#include "src/loadingscreen/LoadingScreen.h"
#include "src/storyDialog.h"
#include "src/dungeon_dialog/DungeonDialog.h"
#include "src/mainMenu.h"
#include "src/theCity.h"

int main(int argc, char *argv[]) {
    // Determine platform based on command line argument
    QString platform = "xcb"; // default for normal run
    for (int i = 0; i < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == "--selftest") {
            platform = "offscreen";
            break;
        }
    }
    qputenv("QT_QPA_PLATFORM", qPrintable(platform));
    QApplication a(argc, argv);

    // Layout probe: build the real DungeonDialog through the normal xcb path,
    // show it, and dump the on-screen geometry of every action button. Used to
    // verify the button grid against what the user actually sees.
    if (a.arguments().contains("--probe-dungeon")) {
        LoadingScreen loadingScreen;
        loadingScreen.exec();
        // Reproduce the REAL style inheritance: in the game DungeonDialog is a
        // child of theCity, a child of GameMenu, and GameMenu applies
        // MainMenu.qss. The sheet must sit on an ANCESTOR, not on the dialog
        // itself, or the dialog's own override would be replaced instead of
        // layered on top. Without this the probe renders plain 32px buttons and
        // misses what the player sees.
        QWidget styleHost;
        {
            QFile qss(QFileInfo(__FILE__).absolutePath() + "/qss/MainMenu.qss");
            if (qss.open(QFile::ReadOnly | QFile::Text))
                styleHost.setStyleSheet(QString::fromUtf8(qss.readAll()));
        }
        styleHost.show();
        DungeonDialog dlg(&styleHost);
        dlg.show();
        // Probe at several sizes: the default resize, the declared minimum
        // (900x640), and the size the user's screen actually gives it.
        QList<QSize> sizes = {QSize(1280, 800), QSize(900, 640), QSize(1636, 1070)};
        for (const QSize& sz : sizes) {
            dlg.resize(sz);
            for (int i = 0; i < 40; ++i) QApplication::processEvents();
            qInfo() << "=== DUNGEON PROBE" << sz << "===";
            qInfo() << "actual dialog size:" << dlg.size();
            QWidget* sidebar = dlg.findChild<QWidget*>("dungeonSidebar");
            if (!sidebar) { qInfo() << "no sidebar!"; continue; }
            qInfo() << "sidebar:" << sidebar->geometry();
            QList<QWidget*> kids = sidebar->findChildren<QWidget*>(
                QString(), Qt::FindDirectChildrenOnly);
            QList<QPair<QString,QRect>> rects;
            for (QWidget* w : kids) {
                if (!w->isVisible() || w->geometry().isEmpty()) continue;
                const QRect r = w->geometry();
                QString label = w->objectName().isEmpty()
                    ? QString(w->metaObject()->className()) : w->objectName();
                if (auto* gb = qobject_cast<QGroupBox*>(w)) label += "(" + gb->title() + ")";
                if (auto* btn = qobject_cast<QPushButton*>(w)) label += "(" + btn->text() + ")";
                if (auto* lbl = qobject_cast<QLabel*>(w)) label += "(" + lbl->text().left(20) + ")";
                rects.append({label, r});
                qInfo().noquote() << QString("  %1  geo=%2,%3 %4x%5  bottom=%6")
                    .arg(label, -40)
                    .arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height())
                    .arg(r.y() + r.height());
            }
            int overlaps = 0;
            for (int i = 0; i < rects.size(); ++i)
                for (int j = i + 1; j < rects.size(); ++j)
                    if (rects[i].second.intersects(rects[j].second)) {
                        ++overlaps;
                        qInfo().noquote() << QString("  OVERLAP: %1 <-> %2")
                            .arg(rects[i].first, rects[j].first);
                    }
            qInfo() << "sidebar direct-child overlaps:" << overlaps;
            int bottom = 0;
            for (auto& p : rects) bottom = qMax(bottom, p.second.bottom());
            qInfo() << "sidebar height:" << sidebar->height()
                    << "content bottom:" << bottom
                    << (bottom > sidebar->height() ? "  <<< CONTENT OVERFLOWS" : "");
            qInfo() << "=== END PROBE" << sz << "===";
            dlg.grab().save(QString("/tmp/probe_%1x%2.png").arg(sz.width()).arg(sz.height()));
        }
        dlg.resize(1280, 800);
        for (int i = 0; i < 40; ++i) QApplication::processEvents();
        dlg.grab().save("/tmp/dungeon_probe.png");
        qInfo() << "saved grab to /tmp/dungeon_probe.png";
        return 0;
    }

    // Menu probe: build the real GameMenu and report whether its background
    // pixmap loaded. Verifies the introtitle.png copy into DESTDIR.
    if (a.arguments().contains("--probe-menu")) {
        GameMenu w;
        w.resize(1280, 800);
        w.show();
        for (int i = 0; i < 40; ++i) QApplication::processEvents();
        const QString p = QDir::cleanPath(qApp->applicationDirPath() + "/introtitle.png");
        QPixmap probe;
        qInfo() << "=== MENU PROBE ===";
        qInfo() << "applicationDirPath:" << qApp->applicationDirPath();
        qInfo() << "introtitle path:" << p << "exists:" << QFile::exists(p);
        qInfo() << "loads as pixmap:" << probe.load(p)
                << "size:" << probe.size();
        w.grab().save("/tmp/menu_probe.png");
        qInfo() << "saved grab to /tmp/menu_probe.png";
        qInfo() << "=== END MENU PROBE ===";
        return 0;
    }

    // Initial sequence
    LoadingScreen loadingScreen;
    loadingScreen.exec();

    storyDialog story;
    story.exec();

    GameMenu w;
    w.show();
    return a.exec();
}
