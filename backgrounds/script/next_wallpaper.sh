#!/bin/bash

COUNTER_FILE="$HOME/.config/backgrounds/script/counter"
SET_SCRIPT="$HOME/.config/backgrounds/script/set_wallpaper.sh"

# Ensure counter file exists
if [[ ! -f "$COUNTER_FILE" ]]; then
    echo "0" > "$COUNTER_FILE"
fi

# Increment counter
COUNTER=$(cat "$COUNTER_FILE")
NEW_COUNTER=$((COUNTER + 1))
echo "$NEW_COUNTER" > "$COUNTER_FILE"

# Call the set_wallpaper script
bash "$SET_SCRIPT"
