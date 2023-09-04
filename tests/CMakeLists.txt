﻿cmake_minimum_required (VERSION 3.20)

set(WDK_INCLUDE_DIRECTORY
    "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0"
)
set(WDK_LIB_DIRECTORY
    "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0"
)
set(STL_INCLUDE_DIRECTORY
    "C:/Program Files/Microsoft Visual Studio/2022/Preview/VC/Tools/MSVC/14.38.32919/include"
)

function(add_kernel_test_file path)
    string(REGEX MATCH "^${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/(.*)\.cpp$" dont_care "${path}")
    set(SLASHED_NAME "${CMAKE_MATCH_1}")
    string(REPLACE "/" "__" UNDER_NAME "${SLASHED_NAME}")

    file(CONFIGURE OUTPUT "${UNDER_NAME}_constants.cpp" CONTENT
        "const wchar_t *NT_DEVICE_NAME = L\"\\\\Device\\\\${SLASHED_NAME}\";\nconst wchar_t *DOS_DEVICE_NAME = L\"\\\\DosDevices\\\\${SLASHED_NAME}\";\n"
    )

    # Add source to this project's executable.
    add_library (${UNDER_NAME}_lib STATIC
        "${UNDER_NAME}_constants.cpp"
        ${path})

    cmake_policy(SET CMP0092 NEW) # Do not add /W3 to CMAKE_<LANG>_FLAGS by default
    target_compile_options(${UNDER_NAME}_lib PRIVATE /EHs-c- /W4 /WX /std:c++latest)

    target_include_directories(${UNDER_NAME}_lib PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        "${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/support"
        ../sys
        ${STL_INCLUDE_DIRECTORY}
        "C:/Program Files (x86)/Windows Kits/10/include/10.0.22621.0/ucrt"
        ${WDK_INCLUDE_DIRECTORY}/km
        ${WDK_INCLUDE_DIRECTORY}/km/crt
        ${WDK_INCLUDE_DIRECTORY}/shared
    )

    target_compile_options(${UNDER_NAME}_lib PRIVATE /kernel /wd5040)
    target_compile_definitions(${UNDER_NAME}_lib PRIVATE
        _SILENCE_CXX20_CISO646_REMOVED_WARNING
        TEST_STD_VER=26
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
    set_target_properties(${UNDER_NAME}_lib PROPERTIES LIBRARY_OUTPUT_DIRECTORY tests)

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
        "/PDB:${UNDER_NAME}.sys.pdb"
        "/OUT:${UNDER_NAME}.sys"
        "../sys/ksys_common.lib"
        "${UNDER_NAME}_lib.lib"
    )

    add_custom_target (${UNDER_NAME}_sys ALL
        ${MY_KLINK_CMD}
        COMMAND signtool.exe sign -q -fd SHA256 -p placeholderPassword -f ../MsvcStlTestingCert.pfx ${UNDER_NAME}.sys > NUL
        DEPENDS ${UNDER_NAME}_lib ksys_common
        BYPRODUCTS "${UNDER_NAME}.sys" "${UNDER_NAME}.sys.pdb")

    add_custom_target (${UNDER_NAME}_asm ALL
        dumpbin /disasm ${UNDER_NAME}.sys /OUT:${UNDER_NAME}.sys.asm > NUL
        DEPENDS ${UNDER_NAME}_sys
        BYPRODUCTS "${UNDER_NAME}.sys.asm")

    add_custom_target (${UNDER_NAME}_asmcheck ALL
        ../asm_checker/asm_checker.exe ${UNDER_NAME}.sys.asm && echo ${UNDER_NAME}.sys.asm > ${UNDER_NAME}.sys.asm.log
        DEPENDS ${UNDER_NAME}_asm asm_checker
        BYPRODUCTS "${UNDER_NAME}.sys.asm.log")
endfunction()

set(TEST_DIRS
    #${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/numerics/numeric.ops
    #${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/numerics/rand/rand.adapt
    #${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/numerics/rand/rand.eng
    #${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/numerics/rand/rand.predef
    #${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/numerics/rand/rand.req
    #${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/numerics/rand/rand.dist/rand.dist.uni/rand.dist.uni.int
)

# Tests MSVC is excluding
set(TEST_EXCLUSIONS
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.copy/ranges.copy.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.copy/ranges.copy_backward.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.copy/ranges.copy_n.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.fill/ranges.fill.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.move/ranges.move.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.move/ranges.move_backward.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.partitions/ranges_partition_copy.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.random.sample/ranges_sample.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.random.shuffle/ranges_shuffle.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.remove/ranges.remove.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.remove/ranges.remove_if.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.replace/ranges.replace.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.rotate/ranges.rotate_copy.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.rotate/ranges_rotate.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.swap/ranges.swap_ranges.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.transform/ranges.transform.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.modifying.operations/alg.unique/ranges_unique_copy.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.nonmodifying/alg.all_of/ranges.all_of.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.nonmodifying/alg.any_of/ranges.any_of.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.nonmodifying/alg.equal/ranges.equal.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.nonmodifying/alg.find.end/ranges.find_end.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.nonmodifying/alg.foreach/ranges.for_each.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.nonmodifying/alg.foreach/ranges.for_each_n.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.nonmodifying/alg.none_of/ranges.none_of.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.nonmodifying/alg.search/ranges.search.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.nonmodifying/alg.search/ranges.search_n.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.nonmodifying/mismatch/ranges_mismatch.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.clamp/ranges.clamp.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.heap.operations/make.heap/ranges_make_heap.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.heap.operations/pop.heap/ranges_pop_heap.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.heap.operations/push.heap/ranges_push_heap.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.heap.operations/sort.heap/ranges_sort_heap.pass
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.merge/inplace_merge_comp.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.merge/ranges_inplace_merge.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.merge/ranges_merge.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.min.max/minmax_init_list_comp.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.nth.element/ranges_nth_element.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.partitions/ranges.is_partitioned.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.set.operations/set.difference/ranges_set_difference.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.set.operations/set.intersection/ranges_set_intersection.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.set.operations/set.symmetric.difference/ranges_set_symmetric_difference.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.set.operations/set.union/ranges_set_union.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.sort/is.sorted/ranges.is_sorted.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.sort/is.sorted/ranges.is_sorted_until.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.sort/partial.sort.copy/ranges_partial_sort_copy.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.sort/partial.sort.copy/ranges_partial_sort_copy.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.sort/partial.sort/partial_sort.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/alg.sorting/alg.sort/partial.sort/partial_sort_comp.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/algorithms.results/in_found_result.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/algorithms.results/min_max_result.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/algorithms.results/no_unique_address.compile.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/ranges_robust_against_dangling.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/robust_against_adl.compile.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/robust_against_proxy_iterators_lifetime_bugs.pass.cpp
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/algorithms/robust_re_difference_type.compile.pass.cpp
)
# Tests I'm excluding
list(APPEND TEST_EXCLUSIONS
    ${CMAKE_SOURCE_DIR}/llvm-project/libcxx/test/std/numerics/numeric.ops/numeric.ops.midpoint/midpoint.float.pass.cpp
)

set(sources "")

foreach(dir IN LISTS TEST_DIRS)
    file(
        GLOB_RECURSE
        local_sources
        LIST_DIRECTORIES false
        CONFIGURE_DEPENDS
        ${dir}/*.cpp
    )
    list(FILTER local_sources EXCLUDE REGEX "\.fail\.cpp$")
    list(APPEND sources "${local_sources}")
endforeach()

list(REMOVE_ITEM sources ${TEST_EXCLUSIONS})

foreach(source IN LISTS sources)
    add_kernel_test_file(${source})
endforeach()