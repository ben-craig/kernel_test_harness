@echo off
setlocal
set CC="C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Tools\MSVC\14.10.25017\bin\HostX64\x64\cl.exe"


set CCFLAGS=^
/nologo                                                                ^
/c                                                                     ^
/W3                                                                    ^
/Zp8                                                                   ^
/Zc:wchar_t                                                            ^
/GS-                                                                   ^
/Zc:forScope                                                           ^
/wd4290                                                                ^
/wo4819                                                                ^
/GR-                                                                   ^
/EHs-c-                                                                ^
/Zl                                                                    ^
/Gz                                                                    ^
/kernel                                                                ^
/Zi                                                                    ^
/Od                                                                    ^
/Oy-                                                                   ^
/D_WIN32_WINNT=0x0601                                                  ^
/DNTDDI_VERSION=0x06010000                                             ^
/D_AMD64_                                                              ^
/DDEPRECATE_DDK_FUNCTIONS                                              ^
/DWIN32                                                                ^
/D_WIN32                                                               ^
/D_WINNT                                                               ^
/D_WIN32_IE=0x0700                                                     ^
/DWINVER=0x0601                                                        ^
/D_AMD64_=1                                                            ^
/DWIN64                                                                ^
/D_WIN64                                                               ^
/DPOOL_NX_OPTIN=1                                                      ^
/DDEBUG                                                                ^
/D "KMDF_VERSION_MAJOR=1"                                              ^
/D "KMDF_VERSION_MINOR=11"                                             ^
/I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.15063.0\km\crt" ^
/I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.15063.0\km"     ^
/I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.15063.0\shared" ^
/I"C:\Program Files (x86)\Windows Kits\10\Include\wdf\kmdf\1.11"

set LINK_PROG="C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Tools\MSVC\14.10.25017\bin\HostX64\x64\link.exe"

set LINK_FLAGS=^
/nologo                                                                      ^
/OPT:REF                                                                     ^
/IGNORE:4089                                                                 ^
/IGNORE:4210                                                                 ^
/DRIVER                                                                      ^
/IGNORE:4078                                                                 ^
/release                                                                     ^
/nodefaultlib                                                                ^
/safeseh:no                                                                  ^
/entry:DriverEntry                                                           ^
/subsystem:native                                                            ^
/machine:x64                                                                 ^
/incremental:no                                                              ^
/OPT:NOICF                                                                   ^
/DEBUGTYPE:cv,fixup                                                          ^
/DEBUG                                                                       ^
/LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.15063.0\km\x64"    ^
/LIBPATH:"C:\Program Files (x86)\Windows Kits\10\lib\wdf\kmdf\x64\1.11"      ^
BufferOverflowFastFailK.lib                                                  ^
ntoskrnl.lib                                                                 ^
hal.lib                                                                      ^
wmilib.lib                                                                   ^
WdfLdr.lib                                                                   ^
WdfDriverEntry.lib

mkdir x64
mkdir x64\Release

%CC% %CCFLAGS% /Fox64\Release\toaster.obj /Fdx64\Release\toaster.pdb toaster.c
%CC% %CCFLAGS% /Fox64\Release\Source.obj /Fdx64\Release\Source.pdb Source.cpp

%LINK_PROG% %LINK_FLAGS% ^
   /PDB:x64\Release\kernel_mode.pdb /OUT:x64\Release\kernel_mode.sys ^
   x64\Release\toaster.obj ^
   x64\Release\Source.obj

copy kernel_mode.inf x64\Release
echo inf2cat
"C:\Program Files (x86)\Windows Kits\10\bin\x86\inf2cat.exe" /drv:x64\Release /os:XP_X64,Server2003_X64,Vista_X64,Server2008_X64
echo signtool
"C:\Program Files (x86)\Windows Kits\10\bin\x86\signtool.exe" sign /f p:\perforce\build\nibuild\export\17.0\17.0.0f3\includes\NITestingCert.pfx /p foo x64\Release\kernel_mode.cat

endlocal
