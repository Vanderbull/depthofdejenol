# Roadmap v6.7.0 — Balance pass, menu buttons follow the Options font

**Goal:** Make level-up progression meaningful and ensure all UI follows the font chosen in Options.

Status: ✅ DONE (eftersläppt)

---

## 1. Font sync — all UI follows Options

**Why:** The main menu and theCity had hard-coded fonts that ignored the user's choice in Options.

**Do:**
- Main menu buttons follow the font chosen in Options
- theCity buttons follow the font chosen in Options
- Button font size comes from Options, not a hard-coded 13pt

**Verify:** Build, run tests, grep for hard-coded font sizes in main menu and theCity.

## 2. Monster XP scaling

**Why:** Leveling was too fast — a level took only ~3 fights, making progression meaningless.

**Do:**
- Scale monster XP 3× so a level takes roughly 10-20 fights

**Verify:** Build, run tests, check XP curve in MonsterBalance.

## 3. Release

**Why:** Version bump and release notes.

**Do:**
- Bump version to 6.7.0 in blacklands.pro
- Update ReleaseInfo.cpp
- Update RELEASE_NOTES.md
- Tag v6.7.0

**Verify:** Full build, full test run (1967 passed, 0 failed).
