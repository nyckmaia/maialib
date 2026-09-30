cmake_minimum_required(VERSION 3.15)

# Enable policy for MSVC runtime library selection
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

# Set MSVC runtime library for Windows (MUST be before project() call)
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Read the library version from the repo-root VERSION file (hand-edited on release).
# NOTE: must happen before project() - ${PROJECT_SOURCE_DIR} does not exist yet,
# so this uses ${CMAKE_CURRENT_SOURCE_DIR} instead (both point at the repo root here).
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" MAIALIB_VERSION)
string(STRIP "${MAIALIB_VERSION}" MAIALIB_VERSION)

# project(... VERSION ...) below accepts only a strict numeric form,
# major[.minor[.patch[.tweak]]], so a value such as "1.11.0-rc1" aborts the
# configure step. Check it here to fail with a message that names the file.
# NOTE: the four optional groups spell out "up to four components" on purpose -
# CMake's regex engine has no brace quantifier, so "{0,3}" would never match.
if(NOT MAIALIB_VERSION MATCHES "^[0-9]+(\\.[0-9]+)?(\\.[0-9]+)?(\\.[0-9]+)?$")
    message(FATAL_ERROR
            "Invalid version '${MAIALIB_VERSION}' in ${CMAKE_CURRENT_SOURCE_DIR}/VERSION: "
            "expected a numeric version such as 1.10.3 - "
            "major[.minor[.patch[.tweak]]], with no suffix.")
endif()

project(maiacore VERSION ${MAIALIB_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(STATIC_LIB "Generate a static libray" OFF)
option(SHARED_LIB "Generate a shared libray" OFF)
option(PYBIND_LIB "Generate a pybind libray" OFF)
option(MAIACORE_BUILD_TESTS "Build the C++ unit tests in tests-cpp (requires STATIC_LIB)" OFF)

if(NOT STATIC_LIB AND NOT SHARED_LIB AND NOT PYBIND_LIB)
    set(STATIC_LIB ON)
endif()

OPTION(PROFILING "Profile functions" OFF)

if(PROFILING)
  add_definitions(-DPROFILING)
endif()

# Debug flags for GCC and Clang, with coverage only on Linux (it causes linking issues on
# Windows with Clang). MSVC-style compilers keep CMake's own Debug flags (/Zi /Ob0 /Od /RTC1):
# cl does not know -g or -O0 (warning D9002).
if(NOT MSVC)
    if(UNIX AND NOT APPLE)
        set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 --coverage")
    else()
        set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    endif()
endif()

if(WIN32)
    set(OS "Windows")
elseif(APPLE)
    set(OS "Darwin")
elseif(UNIX AND NOT APPLE)
    set(OS "Linux")
else()
    message(FATAL_ERROR "Unable to detect the OS")
endif()

# Pass the version to the compiler as a preprocessor definition
# NOTE: unquoted on purpose. py_maiacore.cpp applies MACRO_STRINGIFY() to this
# macro, which already turns the raw token into a quoted string literal. If we
# quote the value here too, MACRO_STRINGIFY() stringifies the quotes themselves,
# so __version__ ends up carrying literal quote characters (e.g. "\"1.10.3\""
# instead of "1.10.3"). This mirrors the unquoted value setup.py passes via
# -DMAIALIB_VERSION_INFO for the pip build path.
add_definitions(-DMAIALIB_VERSION_INFO=${MAIALIB_VERSION})

# Set otimization flags
if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /W3")
    add_definitions("/wd4244 /wd4267")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    # Set iterator debug level to 0 on Windows for compatibility with tests
    if(WIN32)
        add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
    endif()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

include(FetchContent)
FetchContent_Declare(
  cpptrace
  GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
  GIT_TAG        v0.8.2 # <HASH or TAG>
)
FetchContent_MakeAvailable(cpptrace)

set(MAIALIB_ROOT_DIR ".")
set(MAIACORE_ROOT_DIR ${MAIALIB_ROOT_DIR}/maiacore)
set(MAIACORE_INCLUDE_DIR ${MAIACORE_ROOT_DIR}/include)

# Add all maiacore source files
file(GLOB maiacore_src
    ${MAIACORE_ROOT_DIR}/src/maiacore/*.cpp
    ${MAIACORE_ROOT_DIR}/src/external/pugi/pugixml.cpp
    ${MAIACORE_INCLUDE_DIR}/external/nlohmann/*.hpp
)

set(LIBS 
    SQLiteCpp
    sqlite3
    cpptrace::cpptrace
)

# The public headers include cpptrace (log.h) and SQLiteCpp (score.h), so the static and
# shared libraries pass both on to whatever links them.

# Generate static library
if(STATIC_LIB)
    message("=====> GENERATING STATIC LIBRARY <=====")
    add_definitions(-DMAIACORE_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${maiacore_src})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
endif()

# Generate shared library
if(SHARED_LIB)
    message("=====> GENERATING SHARED LIBRARY <=====")
    add_definitions(-DMAIACORE_SHARED_LIB)
    add_library(${PROJECT_NAME} SHARED ${maiacore_src})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
    # maiacore declares no export macros: on Windows the DLL exports every symbol of its own
    # objects, or it would export nothing to link against.
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Generate python module library
if(PYBIND_LIB)
    # Minimal supported SDK version
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

    add_subdirectory(maiacore/external/pybind11 SYSTEM)

    # Add all python_wrapper source files
    file(GLOB python_wrapper_src
        ${PROJECT_SOURCE_DIR}/maiacore/src/maiacore/python_wrapper/*.cpp
    )

    add_definitions(-DMAIACORE_PYTHON_MODULE)

    pybind11_add_module(${PROJECT_NAME} ${maiacore_src} ${python_wrapper_src})

    message("=====> GENERATING PYBIND LIBRARY <=====")
    if(WIN32)
        if(MSYS OR MINGW OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
            message(STATUS "===> Selected Windows Compiler: MSYS or MINGW or Clang")
            if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
                # target_link_libraries(${PROJECT_NAME} PRIVATE -static -fuse-ld=lld SQLiteCpp sqlite3 cpptrace::cpptrace)
                target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
            else()
                target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
            endif()
        elseif(MSVC)
            message(STATUS "===> Selected Windows Compiler: MSVC")

            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT") 
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")

            set(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG} /MTd")
            set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

            set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE} /MT")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

            target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
        else()
            message(FATAL_ERROR "===> UNABLE TO FIND A WINDOWS COMPILER")
        endif()
    else()
        # Linux e MacOSX
        target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
    endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${MAIACORE_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${MAIACORE_INCLUDE_DIR}/external)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${MAIACORE_INCLUDE_DIR}/external/pugi)
add_subdirectory(maiacore/external/sqlitecpp SYSTEM)

if(MAIACORE_BUILD_TESTS)
    if(NOT STATIC_LIB)
        message(FATAL_ERROR "MAIACORE_BUILD_TESTS requires STATIC_LIB=ON: the tests link the static maiacore library.")
    endif()
    enable_testing()
    add_subdirectory(tests-cpp)
endif()
