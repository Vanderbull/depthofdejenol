#!/usr/bin/env python3
"""Generate the four seed characters used for testing.

These are checked into data/characters/ (everything else there is gitignored)
so a fresh clone always has a party to load. Re-run this script to restore
them if a test or a manual session deletes them:

    python3 tools/seed_characters.py
"""
import json
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "data", "characters")

RATION = {"id": 366, "name": "Rations", "charges": 0,
          "equipped": False, "identified": True}
SWORD = {"id": 1, "name": "Gnarled Hands", "charges": 0,
         "equipped": False, "identified": True}


def char(name, race, level, hp, maxhp, stats, guild=None, glevel=1,
         gold=0, alive=True, spouse="", children=None, x=17, y=12, lvl=1,
         inv=None, in_city=False):
    s = dict(zip(["Strength", "Intelligence", "Wisdom", "Constitution",
                  "Charisma", "Dexterity"], stats))
    return {
        "Age": 18 + level * 3,
        "Alignment": "Neutral",
        "BankInventory": [],
        "Charisma": s["Charisma"],
        "Children": children or [],
        "Constitution": s["Constitution"],
        "Dexterity": s["Dexterity"],
        "DungeonLevel": lvl,
        "DungeonX": x,
        "DungeonY": y,
        "Equipped": [],
        "Experience": (level - 1) * 100,
        "Gold": gold,
        "GuildExperience": {guild: (level - 1) * 100} if guild else {},
        "GuildLevels": {guild: glevel} if guild else {},
        "HP": hp,
        "Hunger": 100,
        "Intelligence": s["Intelligence"],
        "Inventory": inv or [],
        "Level": level,
        "Mana": s["Wisdom"] * 2,
        "MaxHP": maxhp,
        "MaxHunger": 100,
        "MaxMana": s["Wisdom"] * 2,
        "Name": name,
        "PlayTimeSeconds": 0,
        "Race": race,
        "Spouse": spouse,
        "StatPoints": 0,
        "StatusFlags": 0,
        "Strength": s["Strength"],
        "Wisdom": s["Wisdom"],
        "isAlive": alive,
        "inCity": in_city,
        "row": 0,
    }


CHARS = {
    # A healthy starting hero in the city — the "normal" case.
    "Aria.json": char("Aria", "Human", 1, 24, 24, [12, 9, 8, 12, 10, 11],
                      guild="Warriors Guild", gold=150, in_city=True,
                      inv=[RATION]),
    # A mid-level Dwarf warrior, currently down in the dungeon.
    "Borin.json": char("Borin", "Dwarf", 5, 48, 60, [16, 7, 9, 15, 6, 10],
                       guild="Warriors Guild", glevel=4, gold=1200,
                       lvl=3, x=8, y=4, inv=[RATION, SWORD]),
    # A married character with children — exercises the lifecycle fields.
    "Cael.json": char("Cael", "Elf", 7, 30, 44, [8, 17, 16, 9, 12, 15],
                      guild="Mages Guild", glevel=6, gold=3400,
                      spouse="Dara", children=["Elowen", "Fenn"],
                      inv=[RATION]),
    # A dead hero — exercises the Morgue / revive path.
    "Dara.json": char("Dara", "Hobbit", 3, 0, 36, [9, 11, 13, 10, 14, 16],
                      guild="Thieves Guild", glevel=2, gold=75, alive=False,
                      spouse="Cael", children=["Elowen", "Fenn"], inv=[]),
}


def main():
    os.makedirs(OUT, exist_ok=True)
    for fn, data in CHARS.items():
        with open(os.path.join(OUT, fn), "w") as f:
            json.dump(data, f, indent=4)
            f.write("\n")
        print("wrote", fn)


if __name__ == "__main__":
    main()
