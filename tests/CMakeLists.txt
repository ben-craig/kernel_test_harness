cmake_minimum_required (VERSION 3.20)

set(WDK_INCLUDE_DIRECTORY
    "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0"
)
set(WDK_LIB_DIRECTORY
    "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0"
)
set(STL_INCLUDE_DIRECTORY
    "C:/Program Files/Microsoft Visual Studio/2022/Preview/VC/Tools/MSVC/14.38.32919/include"
)

set(DEV_NAME "dummy.cpp")

file(CONFIGURE OUTPUT "dummy_constants.cpp" CONTENT
    "const wchar_t *NT_DEVICE_NAME = L\"\\\\Device\\\\${DEV_NAME}\";\nconst wchar_t *DOS_DEVICE_NAME = L\"\\\\DosDevices\\\\${DEV_NAME}\";\n"
)

# Add source to this project's executable.
add_library (dummy_test_obj STATIC
    "dummy_constants.cpp"
    "dummy.cpp")

cmake_policy(SET CMP0092 NEW) # Do not add /W3 to CMAKE_<LANG>_FLAGS by default
target_compile_options(dummy_test_obj PRIVATE /EHs-c- /W4 /WX /std:c++latest)

target_include_directories(dummy_test_obj PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ../sys

    ${STL_INCLUDE_DIRECTORY}
    "C:/Program Files (x86)/Windows Kits/10/include/10.0.22621.0/ucrt"
    ${WDK_INCLUDE_DIRECTORY}/km
    ${WDK_INCLUDE_DIRECTORY}/km/crt
    ${WDK_INCLUDE_DIRECTORY}/shared
)

target_compile_options(dummy_test_obj PRIVATE /kernel /wd5040)
target_compile_definitions(dummy_test_obj PRIVATE
    _WIN32_WINNT=0x0601
    NTDDI_VERSION=0x06010000
    _AMD64_
    DEPRECATE_DDK_FUNCTIONS
    _WIN32
    _WINNT
    _WIN32_IE=0x0700
    WINVER=0x0601
    _AMD64_=1
    WIN64
    _WIN64
    POOL_NX_OPTIN=1
    DEBUG
)
set_target_properties(dummy_test_obj PROPERTIES LIBRARY_OUTPUT_DIRECTORY tests)

set(MY_KLINK_CMD
    "link"
    "/nologo"
    "/OPT:REF"
    "/IGNORE:4089"
    "/IGNORE:4210"
    "/DRIVER"
    "/IGNORE:4078"
    "/IGNORE:4001"
    "/release"
    "/nodefaultlib"
    "/safeseh:no"
    "/entry:DriverEntry"
    "/subsystem:native"
    "/machine:x64"
    "/incremental:no"
    "/OPT:NOICF"
    "/DEBUGTYPE:cv,fixup"
    "/DEBUG"
    "/LIBPATH:\"${WDK_LIB_DIRECTORY}/km/x64\""
    "BufferOverflowFastFailK.lib"
    "ntoskrnl.lib"
    "hal.lib"
    "wmilib.lib"
    "Ntstrsafe.lib"
    "/PDB:dummy.sys.pdb"
    "/OUT:dummy.sys"
    "../sys/ksys_common.lib"
    "dummy_test_obj.lib"
)

add_custom_target (dummy_test_original_sys ALL
    ${MY_KLINK_CMD}
    COMMAND signtool.exe sign -q -fd SHA256 -p placeholderPassword -f ../MsvcStlTestingCert.pfx dummy.sys
    DEPENDS dummy_test_obj ksys_common
    BYPRODUCTS "dummy.sys" "dummy.sys.pdb")
