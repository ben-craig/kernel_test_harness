cmake_minimum_required (VERSION 3.20)

project ("kernel_test_harness")

#enable_testing()

# Include sub-projects.
add_subdirectory ("asm_checker")
add_subdirectory ("exe")
add_subdirectory ("sys")
