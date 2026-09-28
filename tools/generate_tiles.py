#!/usr/bin/env python3
"""Generate missing dungeon tiles for Depth of Dejenol."""
from PIL import Image, ImageDraw
import os, math, random

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), 
                   "resources/images")
os.makedirs(OUT, exist_ok=True)

def save(filename, img):
    path = os.path.join(OUT, filename)
    if os.path.isfile(path) and os.path.getsize(path) > 0:
        return False
    img.save(path)
    print(f"  CREATED {filename} ({img.width}x{img.height})")
    return True

def new_img(w, h):
    return Image.new('RGBA', (w, h), (0, 0, 0, 0))

# ---------- Floor tiles (16x16) ----------

def floor_sand(img, w, h):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w-1, h-1], fill=(210, 180, 140, 255))
    random.seed(42)
    for _ in range(60):
        x = random.randint(0, w-2)
        y = random.randint(0, h-2)
        c = random.randint(190, 225)
        d.point((x, y), fill=(c, c-25, c-45, 255))

def floor_stone(img, w, h):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w-1, h-1], fill=(100, 100, 110, 255))
    d.line([0, h//2, w-1, h//2], fill=(130, 130, 140, 255), width=1)
    d.line([w//2, 0, w//2, h-1], fill=(130, 130, 140, 255), width=1)
    d.line([0, 0, w-1, 0], fill=(80, 80, 90, 255), width=1)
    d.line([0, h-1, w-1, h-1], fill=(80, 80, 90, 255), width=1)
    d.line([0, 0, 0, h-1], fill=(80, 80, 90, 255), width=1)
    d.line([w-1, 0, w-1, h-1], fill=(80, 80, 90, 255), width=1)

def floor_water(img, w, h):
    d = ImageDraw.Draw(img)
    for y in range(h):
        shade = 40 + int(math.sin(y * 0.5) * 15)
        d.line([0, y, w-1, y], fill=(30, shade, 120+shade//2, 200))

# ---------- Door (16x24) ----------

def door(img, w, h):
    d = ImageDraw.Draw(img)
    # Outer frame
    d.rectangle([0, 0, w-1, h-1], fill=(60, 30, 10, 255))
    # Inner area
    d.rectangle([2, 2, w-3, h-3], fill=(100, 55, 25, 255))
    # Panels (only draw if there's room)
    panel_w = (w - 8) // 2
    panel_h = (h - 8) // 2
    if panel_w >= 2 and panel_h >= 2:
        # Top-left panel
        d.rectangle([4, 4, 4+panel_w, 4+panel_h], outline=(50, 25, 5, 255), width=1)
        # Top-right panel
        d.rectangle([w-4-panel_w, 4, w-4, 4+panel_h], outline=(50, 25, 5, 255), width=1)
        # Bottom-left
        d.rectangle([4, h-4-panel_h, 4+panel_w, h-4], outline=(50, 25, 5, 255), width=1)
        # Bottom-right
        d.rectangle([w-4-panel_w, h-4-panel_h, w-4, h-4], outline=(50, 25, 5, 255), width=1)
    # Door knob
    d.ellipse([w-9, h//2-3, w-4, h//2+3], fill=(200, 170, 80, 255))
    d.ellipse([w-7, h//2-1, w-5, h//2+1], fill=(150, 120, 50, 255))

# ---------- Dig / Pickaxe (16x16) ----------

def dig(img, w, h):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w-1, h-1], fill=(30, 30, 30, 255))
    # Handle
    d.line([w//2, h//4, w//2, 3*h//4], fill=(140, 90, 40, 255), width=3)
    # Head
    hw = min(8, w//2 - 2)
    d.polygon([
        (w//2-hw, h//4),
        (w//2+hw, h//4),
        (w//2+hw+3, h//4-5),
        (w//2+2, h//4-8),
        (w//2-2, h//4-8),
        (w//2-hw-3, h//4-5),
    ], fill=(190, 190, 190, 255))
    d.polygon([
        (w//2-hw, h//4),
        (w//2+hw, h//4),
        (w//2+hw+3, h//4-5),
        (w//2+2, h//4-8),
        (w//2-2, h//4-8),
        (w//2-hw-3, h//4-5),
    ], outline=(100, 100, 100, 255), width=1)

# ---------- Face sprites (16x20) ----------

def face_north(img, w, h):
    """Front view"""
    d = ImageDraw.Draw(img)
    cx, cy = w//2, h//2
    # Hair
    d.ellipse([cx-11, cy-14, cx+11, cy-2], fill=(40, 25, 10, 255))
    # Head
    d.ellipse([cx-10, cy-10, cx+10, cy+10], fill=(225, 185, 145, 255))
    # Eyes
    d.ellipse([cx-6, cy-5, cx-3, cy-2], fill=(40, 30, 20, 255))
    d.ellipse([cx+3, cy-5, cx+6, cy-2], fill=(40, 30, 20, 255))
    # Nose
    d.polygon([(cx-2, cy-1), (cx+2, cy-1), (cx, cy+3)], fill=(200, 160, 120, 255))
    # Mouth
    d.arc([cx-4, cy+2, cx+4, cy+6], 0, 180, fill=(180, 80, 80, 255), width=1)

def face_south(img, w, h):
    """Back of head"""
    d = ImageDraw.Draw(img)
    cx, cy = w//2, h//2
    # Hair (longer at back)
    d.ellipse([cx-11, cy-14, cx+11, cy+2], fill=(35, 20, 5, 255))
    d.ellipse([cx-10, cy-10, cx+10, cy+10], fill=(185, 145, 105, 255))
    # Hair detail
    d.line([cx-8, cy-12, cx-6, cy-4], fill=(25, 15, 0, 255), width=1)
    d.line([cx+8, cy-12, cx+6, cy-4], fill=(25, 15, 0, 255), width=1)

def face_east(img, w, h):
    """Profile facing right"""
    d = ImageDraw.Draw(img)
    cx, cy = w//2, h//2
    # Hair
    d.ellipse([cx-9, cy-13, cx+7, cy-3], fill=(40, 25, 10, 255))
    # Head (profile)
    d.ellipse([cx-8, cy-10, cx+8, cy+10], fill=(225, 185, 145, 255))
    # Eye
    d.ellipse([cx+3, cy-6, cx+7, cy-2], fill=(40, 30, 20, 255))
    d.ellipse([cx+4, cy-5, cx+6, cy-3], fill=(255, 255, 255, 255))
    # Nose
    d.polygon([(cx+6, cy-2), (cx+10, cy), (cx+6, cy+2)], fill=(200, 160, 120, 255))
    # Mouth
    d.arc([cx+3, cy+3, cx+8, cy+7], 0, 180, fill=(180, 80, 80, 255), width=1)
    # Ear
    d.ellipse([cx-7, cy-3, cx-4, cy+3], fill=(210, 170, 130, 255))

def face_west(img, w, h):
    """Profile facing left"""
    d = ImageDraw.Draw(img)
    cx, cy = w//2, h//2
    d.ellipse([cx-7, cy-13, cx+9, cy-3], fill=(40, 25, 10, 255))
    d.ellipse([cx-8, cy-10, cx+8, cy+10], fill=(225, 185, 145, 255))
    d.ellipse([cx-7, cy-6, cx-3, cy-2], fill=(40, 30, 20, 255))
    d.ellipse([cx-6, cy-5, cx-4, cy-3], fill=(255, 255, 255, 255))
    d.polygon([(cx-6, cy-2), (cx-10, cy), (cx-6, cy+2)], fill=(200, 160, 120, 255))
    d.arc([cx-8, cy+3, cx-3, cy+7], 0, 180, fill=(180, 80, 80, 255), width=1)
    d.ellipse([cx+4, cy-3, cx+7, cy+3], fill=(210, 170, 130, 255))

# ---------- Filter tiles (16x16) ----------

def filter_sand(img, w, h):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w-1, h-1], fill=(160, 140, 100, 200))
    d.ellipse([w//4, h//4, 3*w//4-1, 3*h//4-1], outline=(100, 80, 50, 150), width=2)
    d.ellipse([w//3, h//3, 2*w//3-1, 2*h//3-1], outline=(120, 100, 60, 100), width=1)

def filter_water(img, w, h):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w-1, h-1], fill=(40, 80, 140, 200))
    d.ellipse([w//4, h//4, 3*w//4-1, 3*h//4-1], outline=(20, 40, 80, 150), width=2)
    d.ellipse([w//3, h//3, 2*w//3-1, 2*h//3-1], outline=(60, 100, 160, 100), width=1)

# ---------- Special tiles (16x16) ----------

def antimagic(img, w, h):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w-1, h-1], fill=(50, 0, 70, 180))
    cx, cy = w//2, h//2
    for i in range(4):
        a = i * math.pi / 2
        r = 3
        x1 = cx + int(math.cos(a)) * r
        y1 = cy + int(math.sin(a)) * r
        x2 = cx + int(math.cos(a+math.pi/4)) * 5
        y2 = cy + int(math.sin(a+math.pi/4)) * 5
        d.line([x1, y1, x2, y2], fill=(160, 60, 190, 220), width=2)
    d.ellipse([cx-2, cy-2, cx+2, cy+2], fill=(200, 100, 220, 255))

def chute(img, w, h):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w-1, h-1], fill=(25, 25, 30, 255))
    cx = w // 2
    d.polygon([(cx, 2), (w-5, h-5), (5, h-5)], fill=(90, 70, 50, 255))
    d.polygon([(cx, 2), (w-5, h-5), (5, h-5)], outline=(210, 190, 110, 255), width=1)
    d.line([cx, 2, cx, 6], fill=(210, 190, 110, 255), width=1)

def extinguisher(img, w, h):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w-1, h-1], fill=(15, 15, 35, 255))
    cx = w // 2
    d.ellipse([cx-5, 3, cx+5, h-4], fill=(210, 50, 50, 255))
    d.ellipse([cx-5, 3, cx+5, h-4], outline=(150, 30, 30, 255), width=1)
    d.rectangle([cx-2, 0, cx+2, 5], fill=(110, 110, 110, 255))
    d.rectangle([cx-1, 5, cx+1, 8], fill=(80, 80, 80, 255))

def rotator(img, w, h):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w-1, h-1], fill=(35, 55, 15, 255))
    cx, cy = w//2, h//2
    d.ellipse([cx-4, cy-4, cx+4, cy+4], fill=(190, 170, 40, 255))
    d.ellipse([cx-2, cy-2, cx+2, cy+2], fill=(140, 120, 20, 255))
    for i in range(8):
        a = i * math.pi / 4
        x1 = cx + int(math.cos(a) * 4)
        y1 = cy + int(math.sin(a) * 4)
        x2 = cx + int(math.cos(a) * 7)
        y2 = cy + int(math.sin(a) * 7)
        d.line([x1, y1, x2, y2], fill=(190, 170, 40, 255), width=2)

def stud(img, w, h):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w-1, h-1], fill=(50, 50, 55, 255))
    cx, cy = w//2, h//2
    d.ellipse([cx-3, cy-3, cx+3, cy+3], fill=(210, 210, 110, 255))
    d.ellipse([cx-1, cy-1, cx+1, cy+1], fill=(240, 240, 150, 255))
    d.line([cx-5, cy, cx+5, cy], fill=(210, 210, 110, 255), width=1)
    d.line([cx, cy-5, cx, cy+5], fill=(210, 210, 110, 255), width=1)

# ---------- Title images ----------

def mordor_art(img, w, h):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w-1, h-1], fill=(25, 10, 35, 255))
    # Corner decorations
    d.rectangle([2, 2, w-3, h-3], outline=(110, 55, 160, 255), width=3)
    d.rectangle([7, 7, w-8, h-8], outline=(65, 35, 95, 255), width=1)
    # Title text
    d.text((w//2-40, h//2-22), "MORDOR", fill=(210, 160, 55, 255))
    d.text((w//2-25, h//2+5), "The Dark Lands", fill=(180, 130, 80, 200))

def introtitle(img, w, h):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w-1, h-1], fill=(8, 4, 18, 255))
    # Top decorative line
    d.line([10, 10, w-10, 10], fill=(200, 150, 50, 200), width=2)
    d.line([10, 14, w-10, 14], fill=(200, 150, 50, 100), width=1)
    # Title
    d.text((15, h//2-35), "THE DEPTHS", fill=(255, 210, 110, 255))
    d.text((25, h//2-5), "OF DEJENOL", fill=(255, 190, 90, 255))
    d.text((50, h//2+25), "BLACK LANDS", fill=(160, 55, 160, 255))
    # Bottom decorative line
    d.line([10, h-14, w-10, h-14], fill=(200, 150, 50, 100), width=1)
    d.line([10, h-10, w-10, h-10], fill=(200, 150, 50, 200), width=2)

# ---------- Dungeon sprites composite (64x64) ----------

def dungeonsprites(img, w, h):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w-1, h-1], fill=(0, 0, 0, 0))
    # Draw 4x4 grid of sample tiles
    tiles = [
        floor_sand, floor_stone, floor_water, door,
        dig, antimagic, chute, extinguisher,
        rotator, stud, filter_sand, filter_water,
        floor_sand, floor_stone, floor_water, door,
    ]
    for idx, tile_fn in enumerate(tiles):
        col = idx % 4
        row = idx // 4
        # Create a tiny tile
        tile = new_img(16, 16)
        tile_fn(tile, 16, 16)
        d._image.paste(tile, (col*16, row*16))

# ---------- Generate all ----------

print("=== Generating missing dungeon tiles ===\n")

specs = [
    ("floor_sand.png", 16, 16, floor_sand),
    ("floor_stone.png", 16, 16, floor_stone),
    ("floor_water.png", 16, 16, floor_water),
    ("door.png", 16, 24, door),
    ("dig.png", 16, 16, dig),
    ("faceeast.png", 16, 20, face_east),
    ("facewest.png", 16, 20, face_west),
    ("facenorth.png", 16, 20, face_north),
    ("facesouth.png", 16, 20, face_south),
    ("filter_sand.png", 16, 16, filter_sand),
    ("filter_water.png", 16, 16, filter_water),
    ("antimagic.png", 16, 16, antimagic),
    ("chute.png", 16, 16, chute),
    ("extinguisher.png", 16, 16, extinguisher),
    ("rotator.png", 16, 16, rotator),
    ("stud.png", 16, 16, stud),
    ("mordor_art.png", 200, 80, mordor_art),
    ("introtitle.png", 400, 100, introtitle),
    ("dungeonsprites.png", 64, 64, dungeonsprites),
]

created = []
for fn, w, h, fn_draw in specs:
    img = new_img(w, h)
    fn_draw(img, w, h)
    if save(fn, img):
        created.append(fn)

print(f"\nCreated {len(created)} new tiles")
total = len([f for f in os.listdir(OUT) if os.path.isfile(os.path.join(OUT, f)) and not f.endswith('~')])
print(f"Total files in {OUT}: {total}")
