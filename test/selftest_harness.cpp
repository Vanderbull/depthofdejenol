#include "selftest_harness.h"
#include <QFile>
#include <QJsonDocument>
#include <cstdio>

int g_passed = 0;
int g_failed = 0;

// Results go straight to stdout, and Qt's own logging is swallowed for the
// duration of the run. The game logs heavily while initialising (full data
// dumps), which would otherwise bury the results and make `make check`
// useless in CI.
void quietMessageHandler(QtMsgType, const QMessageLogContext&, const QString&) {}

void out(const QString& line)
{
    std::fputs(qPrintable(line), stdout);
    std::fputc('\n', stdout);
    std::fflush(stdout);
}

void check(bool condition, const QString& what, const QString& detail)
{
    if (condition) {
        ++g_passed;
        out("  PASS  " + what);
    } else {
        ++g_failed;
        QString line = "  FAIL  " + what;
        if (!detail.isEmpty()) line += "  [" + detail + "]";
        out(line);
    }
}

void section(const QString& title)
{
    out("");
    out(title);
}

const QString kTestSave = "selftest_tmp";

QString savePath() { return "data/saves/" + kTestSave + ".json"; }

// Reads the save file we just wrote. Returns an empty object if unreadable.
QJsonObject readSaveFile()
{
    QFile f(savePath());
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(f.readAll()).object();
}
