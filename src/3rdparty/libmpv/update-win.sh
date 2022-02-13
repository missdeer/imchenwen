#!/bin/bash
libmpv64=`curl -s -L https://sourceforge.net/projects/mpv-player-windows/files/libmpv/ | grep -o 'https:\/\/sourceforge\.net\/projects\/mpv\-player\-windows\/files\/libmpv\/mpv\-dev\-x86_64\-[0-9]\+\-git\-[0-9a-z]\+\.7z\/download' | sort -n | uniq | tail -n 1`
echo "save $libmpv64 as libmpv-dev-x86_64.7z"
curl -L -o x86_64/libmpv-dev-x86_64.7z $libmpv64
libmpv32=`curl -s -L https://sourceforge.net/projects/mpv-player-windows/files/libmpv/ | grep -o 'https:\/\/sourceforge\.net\/projects\/mpv\-player\-windows\/files\/libmpv\/mpv\-dev\-i686\-[0-9]\+\-git\-[0-9a-z]\+\.7z\/download' | sort -n | uniq | tail -n 1`
echo "save $libmpv32 as libmpv-dev-i686.7z"
curl -L -o i686/libmpv-dev-i686.7z $libmpv32
mpv32=`curl -s -L https://sourceforge.net/projects/mpv-player-windows/files/32bit/ | grep -o 'https:\/\/sourceforge\.net\/projects\/mpv\-player\-windows\/files\/32bit\/mpv\-i686\-[0-9]\+\-git\-[0-9a-z]\+\.7z\/download' | sort -n | uniq | tail -n 1`
echo "save $mpv32 as mpv-i686.7z"
curl -L -o mpv-i686.7z $mpv32
mpv64=`curl -s -L https://sourceforge.net/projects/mpv-player-windows/files/64bit/ | grep -o 'https:\/\/sourceforge\.net\/projects\/mpv\-player\-windows\/files\/64bit\/mpv\-x86_64\-[0-9]\+\-git\-[0-9a-z]\+\.7z\/download' | sort -n | uniq | tail -n 1`
echo "save $mpv64 as mpv-x86_64.7z"
curl -L -o mpv-x86_64.7z $mpv64
