# Roadmap v6.8.0 — Combat feedback, orphaned systems, and QoL

**Why:** The arena was removed, v6.7.0 is done, and the codebase has accumulated orphaned systems and missing combat feedback that need attention.

Status: ✅ DONE

---

## 1. Orphaned systems audit

**Why:** NetworkManager and LoadingScreen may be dead code — tested but never called from production code.

**Do:**
- Grep for external callers of NetworkManager and LoadingScreen
- If orphaned: delete the files, remove from .pro, remove tests
- If partially wired: wire them into the game flow

**Verify:** Build, run tests, confirm zero references to deleted classes.

## 2. Combat visual feedback

**Why:** Dungeon combat lacks visual feedback — no flash on damage, no animation on kill.

**Do:**
- Add red flash on party HP when damaged
- Add green flash on monster when killed
- Add combat animation timer (like DungeonDialog's m_combatFxTimer)

**Verify:** Build, run tests, add test that asserts flash state changes.

## 3. Spell selection dialog in combat

**Why:** Combat spell button uses hard-coded "Fireball" instead of letting the player choose.

**Do:**
- Open SpellCastingDialog from combat spell button
- Pass selected spell to combat action
- Disable spell button when no spells known

**Verify:** Build, run tests, add test that spell selection changes the spell used.

## 4. Item selection dialog in combat

**Why:** Combat item button uses a heuristic to find healing items instead of letting the player choose.

**Do:**
- Open item selection dialog from combat item button
- Pass selected item to combat action
- Disable item button when inventory is empty

**Verify:** Build, run tests, add test that item selection changes the item used.

## 5. Monster status effects in target list

**Why:** Target list shows only name and HP — poison/fire/blind status is invisible.

**Do:**
- Show status icons in target list
- Update target list when status changes

**Verify:** Build, run tests, add test that status icons appear.

## 6. Wave bonus (if arena returns)

**Why:** No reward for fast or no-damage clears.

**Do:**
- Bonus points for clearing a wave quickly
- Bonus points for no-damage clears

**Verify:** Build, run tests, add test that bonus is applied.

## 7. Auto-continue option

**Why:** Player must manually click "Next Wave" after each clear.

**Do:**
- Add auto-continue option in Options
- When enabled, automatically start next wave after clear

**Verify:** Build, run tests, add test that auto-continue works.

---

## Release criteria

- [ ] All orphaned systems resolved
- [ ] Combat visual feedback implemented
- [ ] Spell selection dialog works
- [ ] Item selection dialog works
- [ ] Monster status visible in target list
- [ ] All tests pass (2001+)
- [ ] Version bumped to 6.8.0
- [ ] RELEASE_NOTES.md updated
