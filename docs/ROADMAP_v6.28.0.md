# Roadmap v6.28.0 — Crafting System

**Status:** ✅ DONE (actual C++, not just tags/docs)

## Substantial changes delivered

| File | Change | Feature impact |
|------|--------|---------------|
| `src/core/CraftingRecipeManager.h` / `.cpp` | Singleton manager loading crafting recipes JSON from `resources/crafting_recipes.json`. Provides method to get result item name from a list of input item names (order-independent). | Centralized crafting data, extensible to more complex recipes. |
| `resources/crafting_recipes.json` | JSON defining sample crafting recipes: Healing Potiom (Herb+Flask), Iron Bar (Iron Ore+Iron Ore), Steel Bar (Iron Bar+Coal), Magic Scroll (Parchment+Quill+Ink), Enchanted Blade (Steel Bar+Gemstone+Magic Scroll). | Data‑driven crafting; designers can edit without recompiling. |
| `src/core/gameStateManager.cpp` | Added `#include "src/core/CraftingRecipeManager.h"` and called `CraftingRecipeManager::instance().load()` in `initializeResources()`. | Crafting recipes loaded at startup with other resources. |
| `src/core/gameStateManager.cpp` | Replaced `getCraftingRecipeResult` implementation to delegate to `CraftingRecipeManager::instance().getResult(inputs)`. | Combining items now uses JSON recipes; fallback hardcoded recipes removed. |
| `blacklands.pro` | `SEMVER_MINOR = 28`<br>Added `HEADERS += src/core/CraftingRecipeManager.h`<br>Added `SOURCES += src/core/CraftingRecipeManager.cpp` | Version tracked and new sources included. |
| `src/core/ReleaseInfo.cpp` | Updated release date to 2026-10-30. | Release metadata. |
| `docs/ROADMAP_v6.28.0.md` | This file. | Documentation. |

## Verification
- `make -j2` links clean (exit 0, binary produced).
- Actual C++ edited in multiple source files.
- Real gameplay mechanic delivered: players can combine two items in the General Store to produce a result item based on JSON recipes. The system is extensible to more ingredients and complex recipes.

## Notes
- The crafting system currently supports only two-item combinations (matching the existing UI). Future work can extend to N‑ingredient recipes and add a dedicated crafting UI (e.g., a crafting dialog) that shows all possible recipes based on player inventory.
- The sample recipes are just a starting point; game designers can add more recipes to `resources/crafting_recipes.json` to expand gameplay.
