
#!/bin/bash

# 1. Configuration
API_KEY="AIzaSyATKnkn-CDy_sq1LCKcN7F-xCdsYVq4gp0"
# ==========================================
# CONFIGURATION
# ==========================================
MODEL="gemini-2.5-flash" # Stable choice for large context
API_VERSION="v1beta"

# ==========================================
# SCRIPT LOGIC
# ==========================================

if [ -z "$1" ]; then
    echo "Usage: ./ask_gemini.sh \"Your question about specific code\""
    exit 1
fi

USER_QUERY="$1"
TMP_CONTEXT=$(mktemp)
TMP_PAYLOAD=$(mktemp)

# Extract a keyword from your query to filter files (e.g., a class name)
# This takes the longest word from your query as a search hint
HINT=$(echo "$USER_QUERY" | tr -d '[:punct:]' | awk '{for(i=1;i<=NF;i++) if(length($i)>max) {max=length($i); word=$i}} END {print word}')

echo "🔍 Filtering for relevant files containing: '$HINT'..."

# 1. Find C++ files that actually mention the keyword
find . -type f \( -name "*.cpp" -o -name "*.h" \) -not -path '*/.*' | xargs grep -l "$HINT" | while read -r file; do
    echo "   Adding: $file"
    echo -e "\n--- FILE: $file ---\n" >> "$TMP_CONTEXT"
    cat "$file" | tr -d '\000' >> "$TMP_CONTEXT"
    echo -e "\n" >> "$TMP_CONTEXT"
done

# Fallback: If no files match the hint, just grab the most recent 5 files
if [ ! -s "$TMP_CONTEXT" ]; then
    echo "⚠️  No matches for '$HINT'. Adding 5 most recently modified files instead..."
    ls -t **/*.{cpp,h} 2>/dev/null | head -n 5 | while read -r file; do
        echo "   Adding: $file"
        echo -e "\n--- FILE: $file ---\n" >> "$TMP_CONTEXT"
        cat "$file" | tr -d '\000' >> "$TMP_CONTEXT"
    done
fi

# 2. Build the JSON
jq -n --arg query "$USER_QUERY" --rawfile context "$TMP_CONTEXT" \
    '{contents: [{parts: [{text: ("CONTEXT:\n" + $context + "\n\nQUESTION:\n" + $query)}]}]}' > "$TMP_PAYLOAD"

# 3. Request
echo "🚀 Sending to Gemini..."
RESPONSE=$(curl -s -X POST "https://generativelanguage.googleapis.com/${API_VERSION}/models/${MODEL}:generateContent?key=${API_KEY}" \
    -H 'Content-Type: application/json' --data-binary "@$TMP_PAYLOAD")

# Clean up
rm "$TMP_CONTEXT" "$TMP_PAYLOAD"

# 4. Result
TEXT_REPLY=$(echo "$RESPONSE" | jq -r '.candidates[0].content.parts[0].text // empty')

if [ -n "$TEXT_REPLY" ] && [ "$TEXT_REPLY" != "null" ]; then
    echo -e "\n--- Gemini's Response ---\n$TEXT_REPLY"
else
    echo "❌ Error: $(echo "$RESPONSE" | jq -r '.error.message // "Quota exceeded or unknown error."')"
fi
