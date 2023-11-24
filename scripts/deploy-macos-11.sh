#!/bin/bash

# Move compiled bundle here
mv build/src/imchenwen.app .
mv 3rdparty/imchenwen-hlsdl imchenwen.app/Contents/MacOS/
if [ -f /usr/local/opt/ffmpeg/bin/ffmpeg ]; then  cp /usr/local/opt/ffmpeg/bin/ffmpeg imchenwen.app/Contents/MacOS/ ; fi
if [ -f /opt/homebrew/opt/ffmpeg/bin/ffmpeg ]; then  cp /opt/homebrew/opt/ffmpeg/bin/ffmpeg imchenwen.app/Contents/MacOS/ ; fi

# Bundle libraries
macdeployqt imchenwen.app -qmldir=src/qml/ -executable=imchenwen.app/Contents/MacOS/imchenwen-hlsdl

# Fix permissions
chown $USER imchenwen.app/Contents/MacOS/*
chmod 755 imchenwen.app/Contents/MacOS/*

# Fix dependencies of homebrewed libraries
ls imchenwen.app/Contents/Frameworks/*.dylib imchenwen.app/Contents/MacOS/* | while read FILENAME; do
    DEPLOYMENT=`otool -L "$FILENAME" | grep -E "(/usr/local|homebrew)"`
    if [ -n "$DEPLOYMENT" ]; then
        echo "Parsing: $FILENAME"
        echo "$DEPLOYMENT" | while read OLD_PATH; do
            OLD_PATH=${OLD_PATH# *}
            OLD_PATH=${OLD_PATH%% *}
            OLD_NAME=${OLD_PATH##*/}
            NEW_PATH="@executable_path/../Frameworks/$OLD_NAME"
            REAL_PATH="imchenwen.app/Contents/Frameworks/$OLD_NAME"
            echo "  Old path: $OLD_PATH"
            echo "  New path: $NEW_PATH"
            if [ -e "$REAL_PATH" ]; then
                echo "  Found: $REAL_PATH"
                install_name_tool -change "$OLD_PATH" "$NEW_PATH" "$FILENAME"
            else
                echo "  Not found"
            fi
            echo "  ----"
        done
        echo ""
    fi
done

# Compress to zip file
zip -9 -r imchenwen_${TRAVIS_TAG#v}_macOS.zip imchenwen.app
