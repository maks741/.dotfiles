#!/bin/bash

WALLPAPER_DIR="$HOME/.config/backgrounds"
COUNTER_FILE="$HOME/.config/backgrounds/script/counter"

# Read counter
if [[ ! -f "$COUNTER_FILE" ]]; then
    echo "0" > "$COUNTER_FILE"
fi
COUNTER=$(cat "$COUNTER_FILE")

# Get list of .png files
FILES=("$WALLPAPER_DIR"/*.{png,jpg})
NUM_FILES=${#FILES[@]}

if (( NUM_FILES == 0 )); then
    echo "No .png files found in $WALLPAPER_DIR"
    exit 1
fi

# Select file by index
INDEX=$(( COUNTER % NUM_FILES ))
SELECTED_FILE="${FILES[$INDEX]}"

# Set wallpaper with randomized transition
RAND=$(( RANDOM % 2 ))
case $RAND in
    0)
        swww img "$SELECTED_FILE" --transition-fps 60 --transition-step 255 --transition-type wipe
        ;;
    1)
        swww img "$SELECTED_FILE" --transition-fps 60 --transition-step 255 --transition-type any
        ;;
esac
