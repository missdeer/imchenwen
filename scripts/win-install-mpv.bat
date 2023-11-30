: Download libmpv
curl -Lo libmpv.7z https://versaweb.dl.sourceforge.net/project/mpv-player-windows/libmpv/mpv-dev-x86_64-v3-20231126-git-6898d57.7z

: Extract libmpv
"C:\Program Files\7-Zip\7z.exe" x -y -olibmpv libmpv.7z

: Produce .lib file
cd libmpv
del /q mpv.def.bak
rename mpv.def mpv.def.bak
echo EXPORTS > mpv.def
type mpv.def.bak >> mpv.def
lib /def:mpv.def /name:libmpv-2.dll /out:mpv.lib /MACHINE:X64
del /q mpv.def
rename mpv.def.bak mpv.def
cd ..
