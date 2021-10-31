#!/bin/sh

QT_BASE_DIR=/c/Qt/6.2.1/msvc2019_64
if [ -e "$QT_BASE_DIR/bin/moc.exe" ]; then
    echo "Found an existing Qt installation at $QT_BASE_DIR"
    exit
fi

DESTDIR="C:\\Qt"
Base_URL="https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt6_621/qt.qt6.621.win64_msvc2019_64/6.2.1-0-202110220822qtbase-Windows-Windows_10-MSVC2019-Windows-Windows_10-X86_64.7z"
Declarative_URL="https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt6_621/qt.qt6.621.win64_msvc2019_64/6.2.1-0-202110220822qtdeclarative-Windows-Windows_10-MSVC2019-Windows-Windows_10-X86_64.7z"
Tools_URL="https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt6_621/qt.qt6.621.win64_msvc2019_64/6.2.1-0-202110220822qttools-Windows-Windows_10-MSVC2019-Windows-Windows_10-X86_64.7z"

# Download
curl -Lo base.7z "$Base_URL"
curl -Lo declarative.7z "$Declarative_URL"
curl -Lo tools.7z "$Tools_URL"

# Extract
[ -d $DESTDIR ] || mkdir $DESTDIR
7z x -o$DESTDIR base.7z
7z x -o$DESTDIR declarative.7z
7z x -o$DESTDIR tools.7z

QT_VERSION=`ls /c/Qt`
printf "[Paths]\nPrefix = .." > /c/Qt/$QT_VERSION/msvc2019_64/bin/qt.conf