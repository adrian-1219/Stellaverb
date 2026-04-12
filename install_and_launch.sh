#!/bin/bash

# Path to your built plugin
PLUGIN_SOURCE="/Users/adrianlam/Desktop/QMUL/ECS7012 MAP/StellaverbVST/Stellaverb/Builds/MacOSX/build/Debug/Stellaverb.vst3"

# Destination folder (system VST3 directory)
PLUGIN_DEST="/Library/Audio/Plug-Ins/VST3"

osascript -e 'tell application "Ableton Live" to quit'

echo "Copying plugin to VST3 folder..."

# Create destination if it doesn't exist
sudo mkdir -p "$PLUGIN_DEST"

# Copy plugin bundle
sudo cp -R "$PLUGIN_SOURCE" "$PLUGIN_DEST"

echo "Plugin installed successfully."

echo "Launching Ableton Live..."

# Launch Ableton (works regardless of version name in most cases)
open -a "Ableton Live 12 Suite"

echo "Done."
