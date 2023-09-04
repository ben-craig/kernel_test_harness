cmake_minimum_required (VERSION 3.20)

set(WDK_INCLUDE_DIRECTORY
    "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0"
)

# Add source to this project's executable.
add_library (ksys_common STATIC
    "sioctl.cpp"
    "sioctl.h"
    "doAssert.cpp"
    "assert.h"
    "cassert")

cmake_policy(SET CMP0092 NEW) # Do not add /W3 to CMAKE_<LANG>_FLAGS by default
target_compile_options(ksys_common PRIVATE /EHs-c- /W4 /WX /std:c++latest)

set_target_properties(ksys_common PROPERTIES LIBRARY_OUTPUT_DIRECTORY sys)

target_include_directories(ksys_common PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${WDK_INCLUDE_DIRECTORY}/km
    ${WDK_INCLUDE_DIRECTORY}/km/crt
    ${WDK_INCLUDE_DIRECTORY}/shared
)

target_compile_options(ksys_common PRIVATE /kernel /wd5040)
target_compile_definitions(ksys_common PRIVATE
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
    _ITERATOR_DEBUG_LEVEL=0
)
