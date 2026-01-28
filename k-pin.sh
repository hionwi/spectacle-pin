#!/bin/bash
TEMP_DIR="/tmp/spectacle_pins"
mkdir -p "$TEMP_DIR"
FILE_PATH="$TEMP_DIR/pin_tmp.png"

if [ -f "$FILE_PATH" ]; then
    # 以后台模式运行你写的看图程序
    ~/Scripts/k-view "$FILE_PATH" &
fi
