﻿cmake_minimum_required (VERSION 3.20)
cmake_policy(SET CMP0091 NEW)
project ("kernel_test_harness")

string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

# Include sub-projects.
add_subdirectory ("src/asm_checker")
add_subdirectory ("src/stl_kernel")
add_subdirectory ("src/stl_kernel_loader")
add_subdirectory ("tests")
