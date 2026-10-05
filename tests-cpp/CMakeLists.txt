# The C++ unit tests. Built only from the repository root with
# -DSTATIC_LIB=ON -DMAIACORE_BUILD_TESTS=ON (see scripts/make-cpp-tests.py): the
# executable links the maiacore target, so every change to the library relinks it,
# and compiler flags, runtime library and dependencies are the library's own.

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "tests-cpp is not a standalone project: configure the repository root "
                        "with -DSTATIC_LIB=ON -DMAIACORE_BUILD_TESTS=ON, or run 'make cpp-tests'.")
endif()

if(WIN32)
    add_definitions(-DGTEST_HAS_PTHREAD=0)
endif()

# GoogleTest must use the static C runtime that the root CMakeLists.txt selects.
set(gtest_force_shared_crt OFF CACHE BOOL
    "Use shared (DLL) run-time lib even when Google Test is built as static lib." FORCE)
add_subdirectory(googletest-1.14.0)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES COMPILE_FLAGS "-static")
endif()

add_executable(cpp-tests
    src/main.cpp
    src/helpers-test.cpp
    src/note-test.cpp
    src/chord-test.cpp
    src/interval-test.cpp
    src/score-test.cpp
    src/part-test.cpp
    src/measure-test.cpp
    src/fraction-test.cpp
    src/duration-test.cpp
    src/key-test.cpp
    src/time-signature-test.cpp
    src/clef-test.cpp
    src/score-collection-test.cpp
    src/barline-test.cpp
    src/utils-test.cpp
    src/config-test.cpp
    src/pitch-test.cpp
    src/pitch-views-test.cpp
    src/melodic-lines-test.cpp
)

# Tests of maiacore's internal functions include their private headers, which live next to the
# library's sources rather than among its public headers.
target_include_directories(cpp-tests PRIVATE ${CMAKE_SOURCE_DIR}/maiacore/src/maiacore)

# The test sources are also checked for extensions to ISO C++.
if(NOT MSVC)
    target_compile_options(cpp-tests PRIVATE -pedantic)
endif()

target_link_libraries(cpp-tests PRIVATE maiacore gtest)
if(MSYS OR MINGW)
    target_link_libraries(cpp-tests PRIVATE gtest_main)
endif()
if(UNIX)
    target_link_libraries(cpp-tests PRIVATE dl)
endif()

# Keep the executable at the top of the build directory (per configuration for
# multi-config generators), where the scripts expect it.
set_target_properties(cpp-tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# The tests open their fixtures by paths relative to the repository root.
add_test(NAME cpp-tests COMMAND cpp-tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_tests_properties(cpp-tests PROPERTIES TIMEOUT 1800)
