#ifndef SELFTEST_HARNESS_H
#define SELFTEST_HARNESS_H

#include <QString>
#include <QJsonObject>
#include <QFile>

// Shared test harness — included by every selftest_*.cpp file.
// The counters and helpers live in selftest_harness.cpp.

extern int g_passed;
extern int g_failed;

void out(const QString& line);
void check(bool condition, const QString& what, const QString& detail = QString());
void section(const QString& title);

// Defined in selftest_harness.cpp; silences Qt's chatty logging during tests.
void quietMessageHandler(QtMsgType, const QMessageLogContext&, const QString&);

QString savePath();
QJsonObject readSaveFile();

// The save-file name used across tests. Defined in selftest_harness.cpp.
extern const QString kTestSave;

#endif
