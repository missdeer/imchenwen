#!/bin/bash

# Move compiled bundle here
mv build/src/imchenwen.app ./
mv build/3rdparty/hlsdl imchenwen.app/Contents/MacOS/
strip imchenwen.app/Contents/MacOS/imchenwen
strip imchenwen.app/Contents/MacOS/hlsdl


