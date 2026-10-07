#!/usr/bin/env python3
"""Build data/bestiary.json from the repo's own monster data.

Two sources, both already in the repo:

  tools/monsterconverter/data/MDATA5.csv  — 401 monsters, 57 columns of raw
      game stats (hits, att, def, levelFound, numGroups, picID, resistances,
      stats, alignment, goldFactor, type, subtype, ability flags).

  other/walkthrough.txt                   — the original game's monster
      chapter. It groups monsters by picture ("All monsters in a group share
      the same picture") and gives each group a category heading, plus prose
      detail for the humanoid/mage/thief groups (Size, Abilities,
      Resistances, average hits, A/D, stats).

The bestiary needs all of it: the CSV has the numbers, the walkthrough has
the category names and the ability/resistance prose that the CSV only stores
as opaque bit flags.

The picture is what ties the two together: MDATA5's picID selects
resources/images/MON<picID>.jpg (or .png for a couple of ids).

Run:  python3 tools/gen_bestiary.py
"""

from __future__ import annotations

import csv
import json
import os
import re
from collections import Counter, defaultdict

ROOT = os.path.normpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
)
CSV_PATH = os.path.join(ROOT, "tools", "monsterconverter", "data", "MDATA5.csv")
WALKTHROUGH = os.path.join(ROOT, "other", "walkthrough.txt")
IMAGES = os.path.join(ROOT, "resources", "images")
OUT_PATH = os.path.join(ROOT, "data", "bestiary.json")

# MDATA5 column -> bestiary field, for the plain integer stats.
STAT_COLUMNS = {
    "hits": "hits",
    "att": "att",
    "def": "def",
    "levelFound": "level",
    "numGroups": "groups",
    "goldFactor": "goldFactor",
    "alignment": "alignment",
    "type": "type",
    "subtype": "subtype",
}

# Resistance columns in display order, with the label the UI shows.
RESISTANCES = [
    ("ResFire", "Fire"),
    ("ResCold", "Cold"),
    ("ResElectric", "Electric"),
    ("ResMind", "Mind"),
    ("ResPoison", "Poison"),
    ("ResDisease", "Disease"),
    ("ResMagic", "Magic"),
    ("ResPhysical", "Physical"),
    ("ResWeapon", "Weapon"),
    ("ResSpell", "Spell"),
    ("ResSpecial", "Special"),
]

STATS = [
    ("StatStr", "Strength"),
    ("StatInt", "Intelligence"),
    ("StatWis", "Wisdom"),
    ("StatCon", "Constitution"),
    ("StatCha", "Charisma"),
    ("StatDex", "Dexterity"),
]

# The ability flags are stored as opaque bitfields. There is no enum for them
# anywhere in the repo (MTypes.h declares specialAttackFlags/specialPropertyFlags
# as plain int32_t, and every monster has specialPropertyFlags == 0), so their
# bits cannot be named without guessing. They are kept as raw numbers and the
# readable ability text comes from the walkthrough instead.
FLAG_COLUMNS = [
    ("specialAttackFlags", "specialAttackFlags"),
    ("specialPropertyFlags", "specialPropertyFlags"),
    ("spellFlags", "spellFlags"),
]

# Lines in the walkthrough that read like a name list but are prose: field
# labels, ability descriptions and resistance values. Filtering these is what
# keeps the category map free of entries like "Can Backstab" or "Cold 25%".
NOT_A_NAME = (
    "Size", "Usually seen", "Avg", "A/D", "Strength", "Constitution",
    "Dexterity", "Intelligence", "Wisdom", "Charisma", "Abilities",
    "Resistances", "First appears", "Average", "Monsters are seperated",
    "Clean-Up", "Can ", "Charm Resistant", "Resistant", "Immune",
    "Regenerate", "Backstab", "None", "Destroy", "Drain", "Poison",
    "Disease", "See invisible", "Breath", "Spell",
)


def to_int(value: str, default: int = 0) -> int:
    try:
        return int(str(value).strip())
    except (TypeError, ValueError):
        return default


# --------------------------------------------------------------------------
# Walkthrough parsing
# --------------------------------------------------------------------------
def split_sections(body: str):
    """Split the monster chapter into {category: [lines]} on dashed headings."""
    sections = {}
    current, buf = None, []
    for line in body.split("\n"):
        heading = re.match(r"^\s*-{4,}\s*([A-Za-z][A-Za-z /\-]*?)\s*-*\s*$", line)
        if heading:
            if current:
                sections[current] = buf
            current, buf = heading.group(1).strip(), []
        elif current is not None:
            buf.append(line)
    if current:
        sections[current] = buf
    return sections


def join_continuations(lines):
    """Merge a wrapped name list back into one logical line.

    The walkthrough wraps long groups across lines with a trailing comma
    ("... Rattlesnake,\\nSnake Servant, ...") and some groups are a single
    ~80-character line. Both must survive, so the join happens before any
    length test.
    """
    out, i = [], 0
    while i < len(lines):
        s = lines[i].strip()
        while s.endswith(",") and i + 1 < len(lines):
            i += 1
            s = s + " " + lines[i].strip()
        out.append(s)
        i += 1
    return out


def looks_like_name_list(s: str) -> bool:
    """True when a line is a comma-separated list of monster names."""
    if not s or ":" in s:
        return False
    if s.startswith(("-", "_", "*", "=", "o=")):
        return False
    if s.startswith(NOT_A_NAME):
        return False
    if "%" in s:
        return False
    return any(c.isalpha() for c in s)


def normalise(name: str) -> str:
    """Casefold and drop trailing punctuation, for tolerant name matching.

    The walkthrough contains typos and case differences against MDATA5
    ("Zbrat"/"ZBrat", "Gargantuan."/"Gargantuan"), and a missed match means a
    monster loses both its category and its prose.
    """
    return re.sub(r"[^a-z0-9]", "", name.casefold())


def build_name_index(csv_names):
    """normalised CSV name -> the real CSV name (only when unambiguous)."""
    seen = defaultdict(set)
    for n in csv_names:
        seen[normalise(n)].add(n)
    return {k: next(iter(v)) for k, v in seen.items() if len(v) == 1}


def parse_walkthrough(csv_names=()):
    """Return (name -> category, name -> prose detail block).

    The monster chapter starts after the sentence about pictures.
    """
    if not os.path.exists(WALKTHROUGH):
        return {}, {}

    text = open(WALKTHROUGH, encoding="utf-8", errors="replace").read()
    marker = "Monsters are seperated by their given picture"
    start = text.find(marker)
    if start < 0:
        return {}, {}
    body = text[start:]
    # The chapter ends where the next section of the walkthrough begins.
    end = body.find("Dungeon Crawling")
    if end > 0:
        body = body[:end]

    sections = split_sections(body)
    canonical = build_name_index(csv_names)

    def resolve(name):
        """Map a walkthrough name onto a real MDATA5 name when possible."""
        return canonical.get(normalise(name), name)

    name_category = {}
    name_detail = {}

    for category, lines in sections.items():
        # --- Pass 1: the name lists, with wrapped groups re-joined.
        for line in join_continuations(lines):
            if not looks_like_name_list(line):
                continue
            for part in line.rstrip(",").split(","):
                name = part.strip()
                if not name or len(name) > 45:
                    continue
                if name.startswith(NOT_A_NAME) or re.match(r"^\d", name):
                    continue
                name_category.setdefault(resolve(name), category)

        # --- Pass 2: the prose blocks (Humanoids/Mages/Thieves only).
        # A block is an unindented monster name on its own line, immediately
        # followed by "Size:". Requiring that adjacency is what keeps the
        # indented continuation lines of an "Abilities:" list ("See Invisible",
        # "Magic Resistant") from being mistaken for monster names.
        for idx, raw in enumerate(lines):
            if raw[:1] in (" ", "\t"):
                continue
            name = raw.strip()
            if not looks_like_name_list(name) or "," in name or len(name) > 45:
                continue
            tail = [l.strip() for l in lines[idx + 1 : idx + 3]]
            if not any(t.startswith("Size:") for t in tail):
                continue

            # A blank line separates the name from its "Size:" line, so find
            # this block's own Size first and only then look for the next
            # block's — scanning from idx+2 would land on this block's own
            # line and produce an empty block.
            first_size = None
            for j in range(idx + 1, min(idx + 5, len(lines))):
                if lines[j].strip().startswith("Size:"):
                    first_size = j
                    break
            if first_size is None:
                continue

            block_end = len(lines)
            for j in range(first_size + 1, min(first_size + 30, len(lines))):
                if lines[j].strip().startswith("Size:"):
                    block_end = j
                    break
            block = lines[first_size:block_end]

            detail = {}
            pending_key = None
            for entry in block:
                if not entry.strip():
                    pending_key = None
                    continue
                # Two prose lines carry no colon and so do not match the
                # "Key: value" shape; name them explicitly.
                if entry.startswith("Usually seen as"):
                    detail["Usually seen as"] = [entry.strip()]
                    pending_key = "Usually seen as"
                    continue
                if entry.startswith("First appears"):
                    detail["First appears"] = [entry.strip()]
                    pending_key = "First appears"
                    continue
                if re.match(r"^[A-Za-z][A-Za-z/ .]{0,20}:", entry):
                    key, _, value = entry.partition(":")
                    key = key.strip()
                    detail.setdefault(key, [])
                    if value.strip():
                        detail[key].append(value.strip())
                    pending_key = key
                elif pending_key and entry.startswith((" ", "\t")):
                    # continuation line, e.g. the second resistance
                    detail[pending_key].append(entry.strip())

            if detail:
                name_detail[resolve(name.rstrip("."))] = detail

    return name_category, name_detail


# --------------------------------------------------------------------------
# Image lookup
# --------------------------------------------------------------------------
def image_index():
    """picID -> relative image path, preferring .jpg like the editors do."""
    found = defaultdict(list)
    if not os.path.isdir(IMAGES):
        return {}
    for entry in os.listdir(IMAGES):
        m = re.fullmatch(r"MON(\d+)\.(jpg|jpeg|png)", entry, re.I)
        if m:
            found[int(m.group(1))].append(entry)

    index = {}
    for pic_id, names in found.items():
        names.sort(key=lambda n: (os.path.splitext(n)[1].lower() != ".jpg", n))
        index[pic_id] = "resources/images/" + names[0]
    return index


# --------------------------------------------------------------------------
# Build
# --------------------------------------------------------------------------
def main():
    with open(CSV_PATH, newline="", encoding="utf-8", errors="replace") as fh:
        rows = list(csv.DictReader(fh))

    csv_names = {r.get("name", "").strip() for r in rows if r.get("name", "").strip()}

    name_category, name_detail = parse_walkthrough(csv_names)
    images = image_index()

    # A picture's category, taken from the monsters the walkthrough filed
    # under each heading. Monsters the walkthrough never mentions inherit
    # whatever category their picture mostly belongs to.
    picture_category = defaultdict(Counter)
    for row in rows:
        name = row.get("name", "").strip()
        pic = to_int(row.get("picID", "0"), -1)
        if name in name_category and pic >= 0:
            picture_category[pic][name_category[name]] += 1
    picture_category = {
        pic: counts.most_common(1)[0][0] for pic, counts in picture_category.items()
    }

    monsters = []
    for row in rows:
        name = (row.get("name") or "").strip()
        if not name:
            continue

        pic = to_int(row.get("picID", "0"), -1)
        category = name_category.get(name) or picture_category.get(pic, "")

        entry = {
            "name": name,
            "id": to_int(row.get("id", "0")),
            "category": category,
            "picture": pic,
            "image": images.get(pic, ""),
        }

        for column, field in STAT_COLUMNS.items():
            entry[field] = to_int(row.get(column, "0"))

        entry["resistances"] = [
            {"name": label, "percent": to_int(row.get(column, "0"))}
            for column, label in RESISTANCES
            if to_int(row.get(column, "0")) != 0
        ]
        entry["stats"] = {
            label: to_int(row.get(column, "0")) for column, label in STATS
        }

        # Opaque bitfields: keep the raw values so nothing is invented.
        entry["flags"] = {
            field: to_int(row.get(column, "0")) for column, field in FLAG_COLUMNS
        }

        # Prose from the walkthrough, when it covers this monster.
        detail = name_detail.get(name, {})
        entry["size"] = (detail.get("Size") or [""])[0]
        entry["walkthroughResistances"] = detail.get("Resistances", [])
        entry["abilitiesText"] = " ".join(detail.get("Abilities", [])).strip()
        entry["walkthroughHits"] = (detail.get("Avg. Hits") or [""])[0]
        entry["walkthroughAD"] = (detail.get("A/D") or [""])[0]
        entry["walkthroughGroup"] = " ".join(detail.get("Usually seen as", [])).strip()

        monsters.append(entry)

    monsters.sort(key=lambda m: (m["level"], m["name"].lower()))

    categories = sorted({m["category"] for m in monsters if m["category"]})

    payload = {
        "source": "tools/monsterconverter/data/MDATA5.csv + other/walkthrough.txt",
        "count": len(monsters),
        "categories": categories,
        "monsters": monsters,
    }

    os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)
    with open(OUT_PATH, "w", encoding="utf-8") as fh:
        json.dump(payload, fh, indent=1, ensure_ascii=False)

    with_image = sum(1 for m in monsters if m["image"])
    with_cat = sum(1 for m in monsters if m["category"])
    print(f"wrote {OUT_PATH}")
    print(f"  monsters        {len(monsters)}")
    print(f"  with image      {with_image}")
    print(f"  with category   {with_cat}")
    print(f"  categories      {len(categories)}: {', '.join(categories)}")
    print(f"  with prose      {sum(1 for m in monsters if m['size'] or m['abilitiesText'])}")

    # Nothing in the CSV may be left without a category: an uncategorised
    # monster is a monster the bestiary cannot group.
    uncategorised = [m["name"] for m in monsters if not m["category"]]
    if uncategorised:
        print(f"  WARNING uncategorised ({len(uncategorised)}): {', '.join(uncategorised)}")
    # Every walkthrough name we kept should be a real monster name.
    stray = sorted(n for n in name_category if n not in csv_names)
    if stray:
        print(f"  WARNING names not in CSV ({len(stray)}): {', '.join(stray[:10])}")


if __name__ == "__main__":
    main()
