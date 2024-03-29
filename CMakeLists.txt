cmake_minimum_required(VERSION 3.4)
project(maiacore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
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

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 --coverage")

# Set otimization flags
if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /W3")
    add_definitions("/wd4244 /wd4267")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

# Add all maiacore source files
file(GLOB maiacore_src
    ${PROJECT_SOURCE_DIR}/maiacore/src/maiacore/*.cpp
    ${PROJECT_SOURCE_DIR}/maiacore/src/external/pugi/pugixml.cpp
)

# Generate static library
if(STATIC_LIB)
    message("=====> GENERATING STATIC LIBRARY <=====")

    add_definitions(-DMAIACORE_STATIC_LIB)

    add_library(${PROJECT_NAME} STATIC ${maiacore_src})

    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/maiacore/include)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/maiacore/include/external)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/maiacore/include/external/pugi)

    add_subdirectory(maiacore/external/sqlitecpp SYSTEM)

    target_link_libraries(${PROJECT_NAME} SQLiteCpp sqlite3 pthread dl)
endif()

# Generate shared library
if(SHARED_LIB)
    message("=====> GENERATING SHARED LIBRARY <=====")

    add_definitions(-DMAIACORE_SHARED_LIB)

    add_library(${PROJECT_NAME} SHARED ${maiacore_src})

    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/maiacore/include)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/maiacore/include/external)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/maiacore/include/external/pugi)

    add_subdirectory(maiacore/external/sqlitecpp SYSTEM)

    target_link_libraries(${PROJECT_NAME} SQLiteCpp sqlite3 pthread dl)
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

    OPTION(PROFILING "Profile functions" OFF)

    if(PROFILING)
      add_definitions(-DPROFILING)
    endif()

    add_definitions(-DMAIACORE_PYTHON_MODULE)

    pybind11_add_module(${PROJECT_NAME} ${maiacore_src} ${python_wrapper_src})

    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/maiacore/include)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/maiacore/include/external)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/maiacore/include/external/pugi)

    add_subdirectory(maiacore/external/sqlitecpp SYSTEM)

    target_compile_definitions(${PROJECT_NAME} PRIVATE MAIALIB_VERSION_INFO=${MAIALIB_VERSION_INFO})

    message("=====> GENERATING PYBIND LIBRARY <=====")
    if(WIN32)
        if(MSYS OR MINGW OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
            message(STATUS "===> Selected Windows Compiler: MSYS or MINGW or Clang")
            if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
                # target_link_libraries(${PROJECT_NAME} PRIVATE -static -fuse-ld=lld -lwinpthread SQLiteCpp sqlite3)
                target_link_libraries(${PROJECT_NAME} PRIVATE -static -fuse-ld=lld SQLiteCpp sqlite3)
            else()
                target_link_libraries(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive SQLiteCpp sqlite3)
            endif()
        elseif(MSVC)
            message(STATUS "===> Selected Windows Compiler: MSVC")

            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT") 
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")

            set(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG} /MTd")
            set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

            set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE} /MT")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

            # set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")

            add_definitions(-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE)

            target_link_libraries(${PROJECT_NAME} PRIVATE SQLiteCpp sqlite3)
        else()
            message(FALTAL "===> UNABLE TO FIND A WINDOWS COMPILER")
        endif()
    else()
        # Linux e MacOSX
        target_link_libraries(${PROJECT_NAME} PRIVATE SQLiteCpp sqlite3 pthread dl)
    endif()
endif()
