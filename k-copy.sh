#!/bin/bash
TEMP_DIR="/tmp/spectacle_pins"
mkdir -p "$TEMP_DIR"
FILE_PATH="$TEMP_DIR/pin_tmp.png"

# 使用 Spectacle 截取区域
spectacle -rbnc -o "$FILE_PATH"
