#!/usr/bin/env python3
"""Orphaned-system check.

A recurring failure mode in this codebase was: a system gets written, tested
against directly in test/selftest.cpp, and then never called by the game. The
tests pass, the feature does not exist. This script guards against that.

For every class listed below it asserts that at least one reference to
``Class::`` exists *outside* the class's own translation unit(s) and outside
the test suite. A class with zero external callers is either dead code to
delete or a system to wire up -- never quietly both.

Usage:  python3 tools/check_orphaned_systems.py
Exit:   0 = every system is wired, 1 = at least one is orphaned.
"""

import re
import sys
from pathlib import Path

# Class name -> source files that *define* it (excluded from the search).
SYSTEMS = {
    "Endgame": ["src/core/Endgame.cpp", "src/core/Endgame.h"],
    "MonsterBalance": ["src/core/MonsterBalance.cpp", "src/core/MonsterBalance.h"],
    "GoldSinks": ["src/core/GoldSinks.cpp", "src/core/GoldSinks.h"],
    "AgingRules": ["src/core/AgingRules.cpp", "src/core/AgingRules.h"],
    "ItemProgression": ["src/items/ItemProgression.cpp", "src/items/ItemProgression.h"],
    "SpellMechanics": ["src/spell_casting/SpellMechanics.cpp", "src/spell_casting/SpellMechanics.h"],
}

# Directories searched for callers, and directories never searched.
SEARCH_DIRS = ["src", "."]
SKIP_PREFIXES = ("test/", "build/", "3rdparty/", "tools/", ".git/")
EXTENSIONS = (".cpp", ".h", ".cc", ".cxx", ".hpp")


def is_candidate(path: Path, root: Path) -> bool:
    rel = path.relative_to(root).as_posix()
    if not rel.endswith(EXTENSIONS):
        return False
    return not rel.startswith(SKIP_PREFIXES)


def find_callers(cls: str, defining: list, root: Path) -> list:
    """Return files (outside the class's own TU and the tests) that use ``cls::``."""
    pattern = re.compile(r"\b" + re.escape(cls) + r"::")
    excluded = {root / d for d in defining}
    hits = []

    seen = set()
    for d in SEARCH_DIRS:
        base = root / d
        if not base.exists():
            continue
        for path in base.rglob("*"):
            if not path.is_file():
                continue
            try:
                rel = path.relative_to(root).as_posix()
            except ValueError:
                continue
            if rel in seen:
                continue
            seen.add(rel)
            if not is_candidate(path, root):
                continue
            if path in excluded:
                continue
            try:
                text = path.read_text(encoding="utf-8", errors="ignore")
            except OSError:
                continue
            if pattern.search(text):
                hits.append(rel)

    return sorted(hits)


def main() -> int:
    root = Path(__file__).resolve().parent.parent
    failures = []

    print("Orphaned-system check")
    print("=" * 60)

    for cls, defining in sorted(SYSTEMS.items()):
        callers = find_callers(cls, defining, root)
        if callers:
            shown = ", ".join(callers[:4])
            if len(callers) > 4:
                shown += f", +{len(callers) - 4} more"
            print(f"  OK      {cls:<18} {len(callers)} caller(s): {shown}")
        else:
            print(f"  ORPHAN  {cls:<18} no callers outside its own file or the tests")
            failures.append(cls)

    print("=" * 60)
    if failures:
        print(f"FAIL: {len(failures)} orphaned system(s): {', '.join(failures)}")
        print("Either wire the system into the game or delete it. Do not leave it")
        print("reachable only from the test suite.")
        return 1

    print(f"PASS: all {len(SYSTEMS)} systems have at least one external caller.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
