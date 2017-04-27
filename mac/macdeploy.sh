#!/bin/bash
#
# Usage: ./macdeploy.sh <full-path-to-macdeployqt>
#
# macdeployqt is usually located in QTDIR/bin/macdeployqt

if [ -z "$1" ]; then
 echo "Required parameter missing for full path to macdeployqt"
 exit 1
fi

MACDEPLOYQT=$1
QTDIR="`dirname $MACDEPLOYQT`/.."
QTPLUGINS="imchenwen.app/Contents/PlugIns"

# copy known, missing, Qt native library plugins into bundle
#
# See:
#  *  http://code.qt.io/cgit/qt/qttools.git/tree/src/macdeployqt/shared/shared.cpp#n1044
#
mkdir -p $QTPLUGINS

FILE="$QTDIR/plugins/iconengines/libqsvgicon.dylib"
if [ -f "$FILE" ]; then
 cp $FILE $QTPLUGINS/
else
 echo "$FILE: No such file"
 exit 1
fi

# run macdeployqt
$MACDEPLOYQT imchenwen.app 

