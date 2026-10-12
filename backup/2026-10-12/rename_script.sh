#!/bin/bash
# Renames classes and filenames in code and .pro files

# 1. Class and symbol replacements
# Using \b to ensure word boundaries
REPLACEMENTS=(
    "AudioManager:audioManager"
    "DataRegistry:dataRegistry"
    "FontManager:fontManager"
    "GameStateManager:gameStateManager"
    "TheCity:theCity"
    "StoryDialog:storyDialog"
)

# 2. Filename includes and references
# (Already moved files, now update references)
FILE_REFS=(
    "AudioManager.h:audioManager.h"
    "DataRegistry.h:dataRegistry.h"
    "FontManager.h:fontManager.h"
    "GameStateManager.h:gameStateManager.h"
    "TheCity.h:theCity.h"
    "StoryDialog.h:storyDialog.h"
    "Version.h:version.h"
    "Version.h.in:version.h.in"
    "AudioManager.cpp:audioManager.cpp"
    "GameStateManager.cpp:gameStateManager.cpp"
    "TheCity.cpp:theCity.cpp"
    "FontManager.cpp:fontManager.cpp"
)

# 3. Header guards
GUARDS=(
    "AUDIOMANAGER_H:audioManager_H"
    "DATAREGISTRY_H:dataRegistry_H"
    "FONTMANAGER_H:fontManager_H"
    "GAMESTATEMANAGER_H:gameStateManager_H"
    "THECITY_H:theCity_H"
    "STORYDIALOG_H:storyDialog_H"
    "VERSION_H:version_H"
)

# Apply renames across all relevant files
FILES=$(find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.pro" -o -name "*.pro.user" -o -name "*.qss" -o -name "*.json" -o -name "*.lua" -o -name "*.txt" -o -name "*.in" \) -not -path "./.git/*" -not -path "./.venv/*" -not -path "./build/*")

for file in $FILES; do
    echo "Processing $file..."
    
    # Apply class renames
    for pair in "${REPLACEMENTS[@]}"; do
        old=${pair%%:*}
        new=${pair##*:}
        sed -i "s/\b$old\b/$new/g" "$file"
    done
    
    # Apply filename references
    for pair in "${FILE_REFS[@]}"; do
        old=${pair%%:*}
        new=${pair##*:}
        sed -i "s/$old/$new/g" "$file"
    done
    
    # Apply header guards (only in .h and .in files)
    if [[ "$file" == *.h || "$file" == *.in ]]; then
        for pair in "${GUARDS[@]}"; do
            old=${pair%%:*}
            new=${pair##*:}
            sed -i "s/\b$old\b/$new/g" "$file"
        done
    fi
done
