#ifndef SELFTEST_H
#define SELFTEST_H

#include <QString>

// Headless verification suite for the core game logic.
//
// Runs without a GUI or user interaction, so it can gate every change:
//
//     make check
//
// Returns 0 when every check passes, 1 otherwise. Add a check here for any
// behaviour a change is supposed to guarantee — a slice without a check is a
// slice that can silently regress.

int runSelfTest();
int runEquipmentTest();

#endif // SELFTEST_H
