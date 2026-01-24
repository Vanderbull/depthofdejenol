#!/bin/bash

# 1. Configuration
API_KEY="AIzaSyATKnkn-CDy_sq1LCKcN7F-xCdsYVq4gp0"
MODEL="gemini-2.5-flash" # You can also use gemini-1.5-pro
API_VERSION="v1beta"

# ==========================================
# SCRIPT LOGIC
# ==========================================

# 1. Check for 'jq' dependency
if ! command -v jq &> /dev/null; then
    echo "Error: 'jq' is not installed. Please install it to parse API responses."
    echo "Mac: brew install jq  |  Linux: sudo apt install jq"
    exit 1
fi

# 2. Check if user provided a question
if [ -z "$1" ]; then
    echo "Usage: ./ask_gemini.sh \"Your question here\""
    exit 1
fi

USER_QUERY="$1"

# 3. Construct the API call
# We use the v1beta endpoint as it's the standard for the newest models
ENDPOINT="https://generativelanguage.googleapis.com/${API_VERSION}/models/${MODEL}:generateContent?key=${API_KEY}"

# 4. Perform the request
RESPONSE=$(curl -s -X POST "$ENDPOINT" \
    -H 'Content-Type: application/json' \
    -d "{
      \"contents\": [{
        \"parts\":[{\"text\": \"$USER_QUERY\"}]
      }]
    }")

# 5. Extract the text response
# We check for the specific path where Gemini stores the text
TEXT_REPLY=$(echo "$RESPONSE" | jq -r '.candidates[0].content.parts[0].text // empty')

# 6. Output handling
if [ -n "$TEXT_REPLY" ]; then
    echo -e "\n--- Gemini's Response ---\n"
    echo "$TEXT_REPLY"
    echo -e "\n------------------------"
else
    # If TEXT_REPLY is empty, something went wrong. 
    # Let's extract the error message from the JSON if it exists.
    ERROR_MSG=$(echo "$RESPONSE" | jq -r '.error.message // "Unknown Error"')
    echo "Error: $ERROR_MSG"
    
    # Check if it was a 404/Model Not Found and offer a hint
    if [[ "$ERROR_MSG" == *"not found"* ]]; then
        echo "Tip: Try changing the MODEL variable to 'gemini-2.5-flash' or 'gemini-1.5-flash' in the script."
    fi
fi
