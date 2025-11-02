#!/bin/bash

# --- Configuration ---
HEX_FILE="MDATA11.HEX"
BINARY_FILE="MDATA11.bin"
HEADER_FILE="MDATA11_Header_Control.csv"
ROOM_FILE="MDATA11_Room_Data.csv"
CONTROL_FILE="MDATA11_Lairs_Teleporters.csv"

# --- Hex Dump Conversion (Bash Only) ---
echo "Converting hex dump text to binary file: $BINARY_FILE"
# Check if the conversion can be done with xxd (preferred) or requires a manual approach
if command -v xxd >/dev/null 2>&1; then
    # Use xxd if available, which is robust
    xxd -r "$HEX_FILE" > "$BINARY_FILE"
else
    # Fallback: manually strip and convert hex data (less robust but standard Bash)
    HEX_STRING=$(awk 'BEGIN{RS=""; FS="[ \t\n]";} {for(i=1;i<=NF;i++) if(length($i)==2 && $i ~ /^[0-9a-fA-F]{2}$/) printf $i}' "$HEX_FILE" | tr -d '\n')
    echo "$HEX_STRING" | xxd -r -p > "$BINARY_FILE"
fi

if [ ! -f "$BINARY_FILE" ]; then
    echo "Error: Failed to create binary file $BINARY_FILE."
    exit 1
fi
echo "Conversion complete. Starting extraction..."

# --- Data Reading Functions (Little Endian) ---

# Function to read a Little-Endian Word (2 bytes)
# $1: offset
read_word() {
    dd if="$BINARY_FILE" bs=1 skip=$1 count=2 2>/dev/null | od -t u2 -A n | tr -d ' '
}

# Function to read a Little-Endian LongWord (4 bytes)
# $1: offset
read_longword() {
    dd if="$BINARY_FILE" bs=1 skip=$1 count=4 2>/dev/null | od -t u4 -A n | tr -d ' '
}

# Function to read 8 raw bytes for the Currency field (hex)
# $1: offset
read_currency_hex() {
    dd if="$BINARY_FILE" bs=1 skip=$1 count=8 2>/dev/null | xxd -p -c 8 | tr -d '\n'
}

# --- Main Extraction Logic ---

CURRENT_OFFSET=0
# Clear output files and write headers
echo "Parameter,Value,Unit" > "$HEADER_FILE"
echo "Room_Index,Area_Number,BitMask_Currency_Hex" > "$ROOM_FILE"
echo "Record_Type,Index,X,Y,Dest_X,Dest_Y,Dest_Z,Monster_Type_LongWord,Monster_ID" > "$CONTROL_FILE"

# --- 1. FILE HEADER ---
NUM_LEVELS=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
echo "Number of Levels,$NUM_LEVELS," >> "$HEADER_FILE"

# Offset Records
for (( i=1; i<=$NUM_LEVELS; i++ )); do
    OFFSET=$(read_word $CURRENT_OFFSET)
    echo "Level $i Start Offset,$OFFSET,Bytes" >> "$HEADER_FILE"
    let CURRENT_OFFSET+=20 # Skip 20 bytes for the offset record
done

# --- 2. LEVEL HEADER RECORD 1 ---
# The first Level Header Record starts at the offset found in the first record: 12468 (0x30B4)
LEVEL1_START_OFFSET=12468
CURRENT_OFFSET=$LEVEL1_START_OFFSET

X_DIM=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
Y_DIM=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
LEVEL_NUM=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
NUM_AREAS=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
NUM_CHUTES=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
NUM_TELEPORTS=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2

echo "X-dimension,$X_DIM,Tiles" >> "$HEADER_FILE"
echo "Y-dimension,$Y_DIM,Tiles" >> "$HEADER_FILE"
echo "Level Number,$LEVEL_NUM," >> "$HEADER_FILE"
echo "Number of Areas,$NUM_AREAS," >> "$HEADER_FILE"
echo "Number Of Chutes,$NUM_CHUTES," >> "$HEADER_FILE"
echo "Number Of Teleports,$NUM_TELEPORTS," >> "$HEADER_FILE"

# --- 3. ROOM DATA RECORDS LEVEL 1 (900 times) ---
NUM_ROOMS=900
for (( i=1; i<=$NUM_ROOMS; i++ )); do
    AREA_NUM=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
    BITMASK_HEX=$(read_currency_hex $CURRENT_OFFSET) ; let CURRENT_OFFSET+=8
    
    # Skip remaining 10 bytes (20 - 2 - 8)
    let CURRENT_OFFSET+=10 
    
    echo "$i,$AREA_NUM,$BITMASK_HEX" >> "$ROOM_FILE"
done

# --- 4. MONSTER LAIR HEADER & DATA ---
NUM_LAIRS=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
echo "Number Of Lairs,$NUM_LAIRS," >> "$HEADER_FILE"

for (( i=1; i<=$NUM_LAIRS; i++ )); do
    MONSTER_TYPE=$(read_longword $CURRENT_OFFSET) ; let CURRENT_OFFSET+=4
    LAIRED_ID=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
    
    # Skip remaining 14 bytes (20 - 4 - 2)
    let CURRENT_OFFSET+=14
    
    # Record_Type,Index,X,Y,Dest_X,Dest_Y,Dest_Z,Monster_Type_LongWord,Monster_ID
    echo "Monster Lair,$i,,,,,,$MONSTER_TYPE,$LAIRED_ID" >> "$CONTROL_FILE"
done

# --- 5. TELEPORTER DATA ---
for (( i=1; i<=$NUM_TELEPORTS; i++ )); do
    X=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
    Y=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
    DEST_X=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
    DEST_Y=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
    DEST_Z=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
    
    # Record_Type,Index,X,Y,Dest_X,Dest_Y,Dest_Z,Monster_Type_LongWord,Monster_ID
    echo "Teleporter,$i,$X,$Y,$DEST_X,$DEST_Y,$DEST_Z,," >> "$CONTROL_FILE"
done

# --- 6. CHUTES DATA ---
# Chute Header (Word) - This should follow the Teleporters, but the data structure description has a separate header
# We must first skip the Teleporter Header (if present) and then the Chute Header.
# Due to the complexity of the format's ordering, we will try to find the CHUTE header data.
# We already read the number of chutes (NUM_CHUTES=1) in the level header.
# Assuming chute records follow teleporters immediately.

# If you had a Chute Header before the records, you'd read it here.
# Since we already have the count, we proceed to the records:
for (( i=1; i<=$NUM_CHUTES; i++ )); do
    X=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
    Y=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
    LEVELS_TO_DROP=$(read_word $CURRENT_OFFSET) ; let CURRENT_OFFSET+=2
    
    # Record_Type,Index,X,Y,Dest_X,Dest_Y,Dest_Z,Monster_Type_LongWord,Monster_ID
    echo "Chute,$i,$X,$Y,,,," >> "$CONTROL_FILE"
done


echo "✅ Bash extraction complete."
echo "Data has been exported to three CSV files in a readable, tabular format:"
echo "1. $HEADER_FILE (File/Level Dimensions)"
echo "2. $ROOM_FILE (900 Room Records)"
echo "3. $CONTROL_FILE (Lairs, Teleporters, Chutes)"
