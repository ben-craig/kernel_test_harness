@echo off
setlocal
set MY_VC_DIR=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.12.25827
set MY_DDK_VER=10.0.16299.0
set MY_DDK_INCLUDE=C:\Program Files (x86)\Windows Kits\10\Include\%MY_DDK_VER%
set MY_DDK_LIB=C:\Program Files (x86)\Windows Kits\10\Lib\%MY_DDK_VER%
set MY_DDK_BIN=C:\Program Files (x86)\Windows Kits\10\bin\%MY_DDK_VER%
set CC="%MY_VC_DIR%\bin\Hostx64\x64\cl.exe"


set KCCFLAGS=^
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
/I"%MY_DDK_INCLUDE%\km\crt" ^
/I"%MY_DDK_INCLUDE%\km"     ^
/I"%MY_DDK_INCLUDE%\shared"

set LINK_PROG="%MY_VC_DIR%\bin\HostX64\x64\link.exe"

set KLINK_FLAGS=^
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
/LIBPATH:"%MY_DDK_LIB%\km\x64"    ^
BufferOverflowFastFailK.lib                                                  ^
ntoskrnl.lib                                                                 ^
hal.lib                                                                      ^
wmilib.lib


mkdir x64
mkdir x64\Release

%CC% %KCCFLAGS% /Fox64\Release\sioctl.obj /Fdx64\Release\sioctl.pdb sys\sioctl.c

%LINK_PROG% %KLINK_FLAGS% ^
   /PDB:x64\Release\sioctl.pdb /OUT:x64\Release\sioctl.sys ^
   x64\Release\sioctl.obj

echo signtool
"%MY_DDK_BIN%\x64\signtool.exe" sign /f C:\src\private_cert\NITestingCert.pfx /p foo x64\Release\sioctl.sys

set UCCFLAGS=^
/nologo                                                                ^
/c                                                                     ^
/W3                                                                    ^
/Zp8                                                                   ^
/GS-                                                                   ^
/wd4290                                                                ^
/wo4819                                                                ^
/GR-                                                                   ^
/EHs-c-                                                                ^
/Zl                                                                    ^
/Gz                                                                    ^
/Zi                                                                    ^
/Od                                                                    ^
/Oy-                                                                   ^
/MD                                                                    ^
/D_WIN32_WINNT=0x0601                                                  ^
/D_AMD64_                                                              ^
/DWIN32                                                                ^
/D_WIN32                                                               ^
/D_WINNT                                                               ^
/DWINVER=0x0601                                                        ^
/D_AMD64_=1                                                            ^
/DWIN64                                                                ^
/D_WIN64                                                               ^
/DDEBUG                                                                ^
/I"."                                                                  ^
/I"%MY_DDK_INCLUDE%\ucrt"   ^
/I"%MY_DDK_INCLUDE%\um"     ^
/I"%MY_DDK_INCLUDE%\shared" ^
/I"%MY_VC_DIR%\include"

set ULINK_FLAGS=^
/nologo                                                                      ^
/OPT:REF                                                                     ^
/IGNORE:4089                                                                 ^
/IGNORE:4210                                                                 ^
/IGNORE:4078                                                                 ^
/release                                                                     ^
/subsystem:CONSOLE                                                           ^
/machine:x64                                                                 ^
/incremental:no                                                              ^
/OPT:NOICF                                                                   ^
/NXCOMPAT                                                                    ^
/DYNAMICBASE                                                                 ^
/DEBUGTYPE:cv,fixup                                                          ^
/DEBUG                                                                       ^
/LIBPATH:"%MY_DDK_LIB%\um\x64"    ^
/LIBPATH:"%MY_DDK_LIB%\ucrt\x64"  ^
/LIBPATH:"%MY_VC_DIR%\lib\x64" ^
msvcrt.lib                                                                   ^
kernel32.lib                                                                 ^
ucrt.lib                                                                     ^
AdvApi32.lib

%CC% %UCCFLAGS% /Fox64\Release\install.obj /Fdx64\Release\install.pdb exe\install.c
%CC% %UCCFLAGS% /Fox64\Release\testapp.obj /Fdx64\Release\testapp.pdb exe\testapp.c

%LINK_PROG% %ULINK_FLAGS% ^
   /PDB:x64\Release\ioctlapp.pdb /OUT:x64\Release\ioctlapp.exe ^
   x64\Release\install.obj ^
   x64\Release\testapp.obj

endlocal
