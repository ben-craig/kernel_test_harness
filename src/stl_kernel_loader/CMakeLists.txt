cmake_minimum_required (VERSION 3.20)

# Add source to this project's executable.
add_executable (stl_kernel_loader
    "install.cpp"
    "install.h"
    "testapp.cpp")

cmake_policy(SET CMP0092 NEW) # Do not add /W3 to CMAKE_<LANG>_FLAGS by default
target_compile_options(stl_kernel_loader PRIVATE /EHsc /W4 /WX /std:c++latest)

target_include_directories(stl_kernel_loader PRIVATE
    ${CMAKE_SOURCE_DIR}/inc
)
