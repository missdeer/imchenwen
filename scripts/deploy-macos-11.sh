#!/bin/bash

# Move compiled bundle here
mv build/src/imchenwen.app .
mv build/3rdparty/hlsdl imchenwen.app/Contents/MacOS/

./deploy-macos.sh

# Compress to zip file
zip -9 -r imchenwen_${TRAVIS_TAG#v}_macOS.zip imchenwen.app
