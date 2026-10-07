#!/usr/bin/env python3
"""Generate the city-dialog icons that are missing from resources/images/.

theCity::loadButtonIcons() asks GameResources for fifteen keys. Seven of them
have no artwork on disk and are not registered in GameResources::loadResources
either, so those buttons render blank:

    tavern  library  quest_board  character_sheet  bestiary  journal  npc

The "fog" key is registered but its file is missing, so it loads as a null
pixmap. This script draws all eight in the same dark-fantasy pixel-art style as
the existing icons (general_store.png, guilds.png, seer.png, ...): a low-res
canvas drawn with hard-edged primitives, dithered shading, a single warm light
source and a vignette, then upscaled with nearest-neighbour so the pixels stay
crisp.

Run:  python3 tools/gen_city_icons.py
"""

from __future__ import annotations

import math
import os
import random

from PIL import Image, ImageDraw

# Image.NEAREST is a valid resampling constant (PIL >= 9.1 also exposes
# Image.Resampling.NEAREST); alias it so type checkers see a real attribute.
NEAREST = getattr(Image, "Resampling", Image).NEAREST

# --------------------------------------------------------------------------
# Canvas
# --------------------------------------------------------------------------
SCALE = 8
W, H = 64, 76
OUT_DIR = os.path.normpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "resources", "images")
)

# --------------------------------------------------------------------------
# Palette — matched to the existing city icons and theCity.qss
# --------------------------------------------------------------------------
BG        = (26, 26, 46, 255)
BG_DEEP   = (16, 16, 28, 255)
WOOD_D    = (44, 29, 18, 255)
WOOD      = (74, 50, 32, 255)
WOOD_L    = (108, 75, 43, 255)
WOOD_HI   = (146, 104, 60, 255)
GOLD_D    = (146, 106, 28, 255)
GOLD      = (240, 192, 64, 255)
GOLD_L    = (255, 232, 152, 255)
PARCH_D   = (146, 124, 84, 255)
PARCH     = (214, 198, 158, 255)
PARCH_L   = (240, 230, 202, 255)
RED_D     = (88, 20, 20, 255)
RED       = (172, 42, 42, 255)
GREEN_D   = (28, 66, 30, 255)
GREEN     = (72, 132, 56, 255)
BLUE_D    = (22, 32, 64, 255)
BLUE      = (58, 88, 158, 255)
METAL_D   = (68, 68, 84, 255)
METAL     = (132, 132, 152, 255)
FLAME_D   = (236, 128, 28, 255)
FLAME     = (255, 206, 96, 255)
FLAME_L   = (255, 246, 206, 255)
INK       = (32, 28, 36, 255)
LEATH_D   = (58, 32, 24, 255)
LEATH     = (98, 56, 40, 255)
LEATH_L   = (134, 82, 58, 255)
VOID      = (8, 8, 14, 255)


# --------------------------------------------------------------------------
# Helpers
# --------------------------------------------------------------------------
def new_canvas():
    """A vertically-graded background plus its draw handle."""
    im = Image.new("RGBA", (W, H), BG)
    d = ImageDraw.Draw(im)
    for y in range(H):
        t = y / (H - 1)
        c = tuple(int(BG_DEEP[i] + (BG[i] - BG_DEEP[i]) * t) for i in range(3)) + (255,)
        d.line([(0, y), (W, y)], fill=c)
    return im, d


def blend(im, x, y, color, alpha):
    if 0 <= x < im.width and 0 <= y < im.height:
        r, g, b, a = im.getpixel((x, y))
        cr, cg, cb = color[:3]
        im.putpixel(
            (x, y),
            (
                int(r + (cr - r) * alpha),
                int(g + (cg - g) * alpha),
                int(b + (cb - b) * alpha),
                a,
            ),
        )


def glow(im, cx, cy, radius, color, peak=0.55):
    """A soft radial light source — the warm candle/lantern feel of the set."""
    for y in range(max(0, int(cy - radius)), min(im.height, int(cy + radius) + 1)):
        for x in range(max(0, int(cx - radius)), min(im.width, int(cx + radius) + 1)):
            dd = math.hypot(x - cx, y - cy)
            if dd <= radius:
                t = 1.0 - dd / radius
                blend(im, x, y, color, peak * t * t)


def vignette(im, start=0.60, dark=0.55):
    """Dithered darkening toward the edges, the way the existing icons shade."""
    w, h = im.size
    cx, cy = w / 2 - 0.5, h / 2 - 0.5
    maxd = math.hypot(cx, cy)
    for y in range(h):
        for x in range(w):
            dd = math.hypot(x - cx, y - cy) / maxd
            if dd > start:
                t = (dd - start) / (1.0 - start)
                if (x + y) % 2 == 0 or t > 0.55:
                    r, g, b, a = im.getpixel((x, y))
                    k = 1.0 - dark * t * t
                    im.putpixel((x, y), (int(r * k), int(g * k), int(b * k), a))


def lighter(c, amount=40):
    return tuple(min(255, v + amount) for v in c[:3]) + (255,)


def darker(c, amount=40):
    return tuple(max(0, v - amount) for v in c[:3]) + (255,)


# --------------------------------------------------------------------------
# Icons
# --------------------------------------------------------------------------
def icon_tavern():
    """A candle-lit table: foaming tankard, bottle, shelf behind."""
    im, d = new_canvas()

    # back shelf
    d.rectangle([2, 10, 61, 13], fill=WOOD_D)
    d.line([(2, 10), (61, 10)], fill=WOOD)
    d.line([(2, 13), (61, 13)], fill=darker(WOOD_D))

    # bottles on the shelf
    d.rectangle([5, 3, 11, 10], fill=GREEN_D)
    d.rectangle([7, 1, 9, 3], fill=GREEN_D)
    d.rectangle([6, 4, 10, 9], fill=GREEN)

    d.rectangle([50, 4, 57, 10], fill=BLUE_D)
    d.rectangle([52, 1, 55, 4], fill=BLUE_D)
    d.rectangle([51, 5, 56, 9], fill=BLUE)

    # candle (the light source)
    d.rectangle([29, 3, 33, 10], fill=PARCH)
    d.rectangle([29, 3, 30, 10], fill=PARCH_L)
    d.ellipse([28, 0, 34, 5], fill=FLAME_D)
    d.ellipse([29, 1, 33, 4], fill=FLAME)
    d.ellipse([30, 2, 32, 3], fill=FLAME_L)

    # table
    d.rectangle([0, 52, 63, 57], fill=WOOD_L)
    d.line([(0, 52), (63, 52)], fill=WOOD_HI)
    d.rectangle([0, 58, 63, 75], fill=WOOD)
    for x in range(0, 64, 9):
        d.line([(x, 58), (x, 75)], fill=WOOD_D)

    # tankard
    d.rectangle([12, 30, 34, 52], fill=METAL_D)
    d.rectangle([14, 32, 32, 50], fill=METAL)
    d.rectangle([16, 36, 30, 48], fill=GOLD_D)
    d.rectangle([18, 38, 28, 46], fill=GOLD)
    d.rectangle([34, 34, 39, 48], fill=METAL_D)
    d.rectangle([36, 37, 39, 45], fill=BG)

    # foam
    d.ellipse([10, 24, 36, 34], fill=PARCH)
    d.ellipse([13, 22, 28, 31], fill=PARCH_L)
    d.ellipse([24, 23, 34, 31], fill=PARCH)

    # bottle
    d.rectangle([46, 34, 57, 52], fill=GREEN_D)
    d.rectangle([48, 36, 55, 50], fill=GREEN)
    d.rectangle([49, 24, 54, 36], fill=GREEN_D)
    d.rectangle([50, 26, 53, 35], fill=GREEN)
    d.rectangle([49, 21, 54, 25], fill=WOOD_L)
    d.rectangle([50, 40, 53, 46], fill=PARCH)

    glow(im, 31, 5, 28, (255, 180, 70), 0.30)
    vignette(im)
    return im


def icon_library():
    """A bookcase with three shelves of spines and a reading candle."""
    im, d = new_canvas()
    random.seed(7)

    d.rectangle([5, 6, 58, 68], fill=WOOD_D)
    d.rectangle([8, 9, 55, 65], fill=VOID)

    for y in (26, 45):
        d.rectangle([8, y, 55, y + 2], fill=WOOD)

    colours = [RED_D, GREEN_D, BLUE_D, GOLD_D, LEATH, METAL_D, RED, BLUE, GREEN]
    for (y0, y1) in ((10, 26), (29, 45), (48, 65)):
        x = 9
        while x < 53:
            w = random.choice([2, 3, 3, 4])
            h = random.choice([12, 14, 15])
            top = max(y0, y1 - h)
            c = random.choice(colours)
            d.rectangle([x, top, x + w, y1 - 1], fill=c)
            d.line([(x, top), (x, y1 - 1)], fill=lighter(c))
            x += w + 1

    # candle on the middle shelf
    d.rectangle([30, 16, 33, 26], fill=PARCH)
    d.rectangle([30, 16, 31, 26], fill=PARCH_L)
    d.ellipse([29, 13, 34, 18], fill=FLAME)
    d.ellipse([30, 14, 33, 17], fill=FLAME_L)

    glow(im, 31, 16, 24, (255, 180, 70), 0.26)
    vignette(im)
    return im


def icon_quest_board():
    """A plank board with pinned parchment notices."""
    im, d = new_canvas()

    d.rectangle([4, 8, 59, 66], fill=WOOD)
    d.rectangle([4, 8, 59, 10], fill=WOOD_L)
    for y in (27, 47, 66):
        d.line([(4, y), (59, y)], fill=WOOD_D)
    d.rectangle([4, 8, 5, 66], fill=WOOD_D)
    d.rectangle([58, 8, 59, 66], fill=WOOD_D)

    notes = [
        (10, 14, 20, 26),
        (35, 13, 19, 24),
        (13, 34, 21, 26),
        (38, 34, 17, 25),
        (24, 52, 18, 12),
    ]
    for (x, y, w, h) in notes:
        d.rectangle([x, y, x + w, y + h], fill=PARCH)
        d.rectangle([x, y, x + w, y + 1], fill=PARCH_L)
        for i in range(3, h - 2, 3):
            d.line([(x + 2, y + i), (x + w - 2, y + i)], fill=PARCH_D)
        d.ellipse([x + w // 2 - 1, y - 1, x + w // 2 + 1, y + 1], fill=RED)

    vignette(im)
    return im


def icon_character_sheet():
    """A parchment character record: portrait, stat lines, wax seal."""
    im, d = new_canvas()

    d.rectangle([11, 4, 52, 70], fill=PARCH_D)
    d.rectangle([13, 6, 50, 68], fill=PARCH)
    d.rectangle([13, 6, 50, 8], fill=PARCH_L)

    # header band
    d.rectangle([16, 10, 47, 13], fill=GOLD_D)
    d.rectangle([16, 10, 47, 11], fill=GOLD)

    # portrait
    d.ellipse([27, 17, 37, 27], fill=INK)
    d.polygon([(22, 47), (26, 29), (38, 29), (42, 47)], fill=INK)
    d.polygon([(24, 33), (40, 33), (42, 47), (22, 47)], fill=(48, 44, 58, 255))

    # stat lines
    for i, y in enumerate(range(51, 67, 3)):
        d.line([(17, y), (46 - (i % 3) * 5, y)], fill=INK)

    # wax seal
    d.ellipse([38, 58, 48, 68], fill=RED_D)
    d.ellipse([40, 60, 46, 66], fill=RED)

    vignette(im)
    return im


def icon_bestiary():
    """An open book: text on the left leaf, a claw slash on the right."""
    im, d = new_canvas()

    d.polygon([(5, 20), (31, 14), (31, 64), (5, 58)], fill=PARCH)
    d.polygon([(58, 20), (33, 14), (33, 64), (58, 58)], fill=PARCH)
    d.polygon([(5, 20), (31, 14), (31, 18), (5, 24)], fill=PARCH_L)
    d.polygon([(58, 20), (33, 14), (33, 18), (58, 24)], fill=PARCH_L)

    # spine and cover edges
    d.polygon([(31, 14), (33, 14), (33, 64), (31, 64)], fill=PARCH_D)
    d.line([(5, 20), (5, 58)], fill=WOOD_D)
    d.line([(58, 20), (58, 58)], fill=WOOD_D)
    d.line([(5, 58), (31, 64)], fill=WOOD_D)
    d.line([(58, 58), (33, 64)], fill=WOOD_D)

    # text, left leaf
    for i in range(6):
        y = 26 + i * 5
        d.line([(9, y), (27, y - 2)], fill=INK)

    # claw slash, right leaf
    for off in (0, 4, 8):
        d.line([(38 + off, 24), (48 + off // 2, 56)], fill=RED_D)
        d.line([(39 + off, 24), (49 + off // 2, 56)], fill=RED)

    vignette(im)
    return im


def icon_journal():
    """A strapped leather journal with a quill."""
    im, d = new_canvas()

    # page block behind the cover
    d.rectangle([16, 12, 52, 66], fill=PARCH_D)
    d.rectangle([16, 12, 50, 64], fill=PARCH)

    # cover
    d.rectangle([10, 8, 44, 68], fill=LEATH_D)
    d.rectangle([12, 10, 42, 66], fill=LEATH)
    d.rectangle([10, 8, 16, 68], fill=LEATH_D)
    d.rectangle([12, 10, 15, 66], fill=LEATH_L)

    # gold corner pieces
    for y in (10, 62):
        d.rectangle([34, y, 42, y + 4], fill=GOLD_D)
        d.rectangle([34, y, 42, y + 1], fill=GOLD)

    # clasp
    d.rectangle([36, 34, 46, 42], fill=GOLD_D)
    d.rectangle([38, 36, 44, 40], fill=GOLD)

    # quill
    d.line([(47, 66), (58, 22)], fill=PARCH)
    d.polygon([(50, 48), (59, 16), (57, 46)], fill=PARCH_L)

    vignette(im)
    return im


def icon_npc():
    """A hooded figure with glowing eyes."""
    im, d = new_canvas()

    d.polygon([(32, 6), (12, 74), (52, 74)], fill=BLUE_D)
    d.polygon([(32, 10), (16, 74), (48, 74)], fill=(30, 40, 76, 255))
    d.polygon([(32, 8), (18, 40), (46, 40)], fill=BLUE_D)
    d.polygon([(32, 12), (21, 38), (43, 38)], fill=(24, 30, 58, 255))

    # face in shadow
    d.ellipse([22, 20, 42, 44], fill=VOID)

    # eyes
    d.rectangle([26, 29, 29, 31], fill=GOLD_L)
    d.rectangle([35, 29, 38, 31], fill=GOLD_L)

    # cloak folds
    d.line([(32, 40), (32, 74)], fill=(16, 22, 44, 255))
    d.line([(24, 46), (20, 74)], fill=(16, 22, 44, 255))
    d.line([(40, 46), (44, 74)], fill=(16, 22, 44, 255))

    glow(im, 32, 30, 18, (255, 200, 90), 0.20)
    vignette(im)
    return im


def icon_fog():
    """A tileable dungeon-fog patch for the GameResources 'fog' key."""
    im = Image.new("RGBA", (64, 64), (20, 20, 30, 235))
    d = ImageDraw.Draw(im)
    random.seed(3)

    for y in range(64):
        for x in range(64):
            if (x + y) % 2 == 0:
                d.point((x, y), fill=(28, 28, 40, 235))
    for _ in range(220):
        d.point(
            (random.randrange(64), random.randrange(64)),
            fill=(42, 42, 58, 235),
        )
    return im


# --------------------------------------------------------------------------
# Entry point
# --------------------------------------------------------------------------
ICONS = {
    "tavern.png": icon_tavern,
    "library.png": icon_library,
    "quest_board.png": icon_quest_board,
    "character_sheet.png": icon_character_sheet,
    "bestiary.png": icon_bestiary,
    "journal.png": icon_journal,
    "npc.png": icon_npc,
    "fog.png": icon_fog,
}


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    for name, fn in ICONS.items():
        im = fn()
        big = im.resize((im.width * SCALE, im.height * SCALE), NEAREST)
        path = os.path.join(OUT_DIR, name)
        big.save(path)
        print(f"wrote {name:20} {big.width}x{big.height}  ({os.path.getsize(path)} bytes)")


if __name__ == "__main__":
    main()
