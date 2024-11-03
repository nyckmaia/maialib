cmake_minimum_required(VERSION 3.6)
project(maiacore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(STATIC_LIB "Generate a static libray" OFF)
option(SHARED_LIB "Generate a shared libray" OFF)
option(PYBIND_LIB "Generate a pybind libray" OFF)

if(NOT STATIC_LIB AND NOT SHARED_LIB AND NOT PYBIND_LIB)
    set(STATIC_LIB ON)
endif()

OPTION(PROFILING "Profile functions" OFF)

if(PROFILING)
  add_definitions(-DPROFILING)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 --coverage")

if(WIN32)
    set(OS "Windows")
elseif(APPLE)
    set(OS "Darwin")
elseif(UNIX AND NOT APPLE)
    set(OS "Linux")
else()
    message(FATAL_ERROR "Unable to detect the OS")
endif()

# Lê a versão do arquivo VERSION
file(READ "${PROJECT_SOURCE_DIR}/VERSION" MAIALIB_VERSION)
string(STRIP "${MAIALIB_VERSION}" MAIALIB_VERSION)
# Passa a versão como uma definição para o compilador
add_definitions(-DMAIALIB_VERSION_INFO="${MAIALIB_VERSION}")

# Set otimization flags
if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /W3")
    add_definitions("/wd4244 /wd4267")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

include(FetchContent)
FetchContent_Declare(
  cpptrace
  GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
  GIT_TAG        v0.6.2 # <HASH or TAG>
)
FetchContent_MakeAvailable(cpptrace)

set(MAIALIB_ROOT_DIR ".")
set(MAIACORE_ROOT_DIR ${MAIALIB_ROOT_DIR}/maiacore)
set(MAIACORE_INCLUDE_DIR ${MAIACORE_ROOT_DIR}/include)

set(MAIACORE_LIB_DIR ${MAIALIB_ROOT_DIR}/build/${OS}/static/Debug)
set(SQLITECPP_LIB_DIR ${MAIACORE_LIB_DIR}/maiacore/external/sqlitecpp)
set(SQLITE_LIB_DIR ${SQLITECPP_LIB_DIR}/sqlite3)

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
    # pthread
    # dl
)

# Generate static library
if(STATIC_LIB)
    message("=====> GENERATING STATIC LIBRARY <=====")
    add_definitions(-DMAIACORE_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${maiacore_src})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
endif()

# Generate shared library
if(SHARED_LIB)
    message("=====> GENERATING SHARED LIBRARY <=====")
    add_definitions(-DMAIACORE_SHARED_LIB)
    add_library(${PROJECT_NAME} SHARED ${maiacore_src})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
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

            add_definitions(-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE)

            target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
        else()
            message(FALTAL "===> UNABLE TO FIND A WINDOWS COMPILER")
        endif()
    else()
        # Linux e MacOSX
        target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
    endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${MAIACORE_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${MAIACORE_INCLUDE_DIR}/external)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${MAIACORE_INCLUDE_DIR}/external/pugi)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${MAIACORE_INCLUDE_DIR}/external/cpptrace/include)
add_subdirectory(maiacore/external/sqlitecpp SYSTEM)
