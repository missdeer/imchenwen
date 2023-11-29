#!/bin/bash
echo "deploying src/imchenwen.app"
~/Qt/6.6.1/macos_static/bin/macdeployqt src/imchenwen.app -qmldir=../src/qml -appstore-compliant
cd src/imchenwen.app/Contents/Frameworks
install_name_tool -change @loader_path/../../../../opt/libpng/lib/libpng16.16.dylib @executable_path/../Frameworks/libpng16.16.dylib libfreetype.6.dylib
cp /opt/homebrew/lib/libsharpyuv.0.dylib ./
install_name_tool -change @rpath/libsharpyuv.0.dylib @executable_path/../Frameworks/libsharpyuv.0.dylib libwebp.7.dylib
install_name_tool -change @rpath/libsharpyuv.0.dylib @executable_path/../Frameworks/libsharpyuv.0.dylib libwebpmux.3.dylib
install_name_tool -change @rpath/libwebp.7.dylib @executable_path/../Frameworks/libwebp.7.dylib libwebpmux.3.dylib
cd -
