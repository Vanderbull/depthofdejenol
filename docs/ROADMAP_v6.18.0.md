# Roadmap v6.18.0 — Headless tests, bestiary persistence, performance

**Status:** ✅ A+B+C+D done

A. Test hang fixed (offscreen guard in .pro check target).
B. Bestiary unlock persistence verified: `BestiaryDialog::recordEncounter()` writes state; load reads from `m_encountered` set.
C. Performance guard: companion count capped at 3 in combat; dungeon rendering limited to visible tiles.
D. Version bumped to 6.18.0; tag pushed.

Verified: `make check` passes with `QT_QPA_PLATFORM=offscreen`; 0 open roadmap refs.
