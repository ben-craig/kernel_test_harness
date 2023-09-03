cmake_minimum_required (VERSION 3.20)

# Add source to this project's executable.
add_executable (asm_checker
    "asm_checker.cpp")

cmake_policy(SET CMP0092 NEW) # Do not add /W3 to CMAKE_<LANG>_FLAGS by default
target_compile_options(asm_checker PRIVATE /EHsc /W4 /WX /std:c++latest)
