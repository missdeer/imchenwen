cd scripts
powershell.exe "-Command" "if((Get-ExecutionPolicy ) -ne 'AllSigned') { Set-ExecutionPolicy -Scope Process Bypass }; & 'H:\Shareware\imchenwen\scripts\win-install-mpv.ps1'"
cd ..

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
