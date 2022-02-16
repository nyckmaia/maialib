cmake_minimum_required(VERSION 3.0)

find_program(buildcache_program buildcache)
if(buildcache_program)
    message("=====> Use BUILDCACHE: yes <=====")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${buildcache_program}")
else()
    message("=====> Use BUILDCACHE: no found <=====")
endif()

project("maiacore" LANGUAGES CXX)

# Windows: If CMake errors -> Desable your antivirus!!

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

option(STATIC_LIB "Generate a static libray" OFF)
option(SHARED_LIB "Generate a shared libray" OFF)
option(PYBIND_LIB "Generate a pybind libray" OFF)

if(NOT STATIC_LIB AND NOT SHARED_LIB AND NOT PYBIND_LIB)
    # message("--------------------")
    # message("=====> You didn't choose any library type, like: ")
    # message("=> STATIC_LIB")
    # message("=> SHARED_LIB")
    # message("=> PYBIND_LIB")
    # message("")
    # message("USAGE: cmake .. -DSTATIC_LIB=ON")
    # message("")
    # message("Setting the default configuration: STATIC_LIB")    
    # message("--------------------")

    set(STATIC_LIB ON)
endif()

# Add all maiacore source files
file(GLOB maiacore_src
    "./src/*.cpp"
    "./external/pugi/src/pugixml.cpp"
)

# Add all python_wrapper source files
file(GLOB python_wrapper_src
    "./src/python_wrapper/*.cpp"
)

OPTION(PROFILING "Profile functions" OFF)

if(PROFILING)
  add_definitions(-DPROFILING)
endif()

# Generate static library
if(STATIC_LIB)
    message("=====> GENERATING STATIC LIBRARY <=====")

    add_subdirectory(external/zip)
    add_subdirectory(external/sqlitecpp)

    add_library(${PROJECT_NAME} STATIC ${maiacore_src})

    target_include_directories(${PROJECT_NAME} PUBLIC include)
    target_include_directories(${PROJECT_NAME} PUBLIC external/pugi/include)
    target_include_directories(${PROJECT_NAME} PUBLIC external)
    target_include_directories(${PROJECT_NAME} PUBLIC external/cherno)

    target_link_libraries(${PROJECT_NAME} zip SQLiteCpp sqlite3 pthread dl)
endif()

# Generate shared library
if(SHARED_LIB)
    message("=====> GENERATING SHARED LIBRARY <=====")
    add_subdirectory(external/zip)
    add_subdirectory(external/sqlitecpp)

    add_library(${PROJECT_NAME} SHARED ${maiacore_src})

    target_include_directories(${PROJECT_NAME} PUBLIC include)
    target_include_directories(${PROJECT_NAME} PUBLIC external/pugi/include)
    target_include_directories(${PROJECT_NAME} PUBLIC external)
    target_include_directories(${PROJECT_NAME} PUBLIC external/cherno)

    target_link_libraries(${PROJECT_NAME} zip SQLiteCpp sqlite3 pthread dl)
endif()

# Generate python module library
if(PYBIND_LIB)
    message("=====> GENERATING PYBIND LIBRARY <=====")
    
    add_definitions(-DPYBIND)
    
    find_package(Python COMPONENTS Interpreter Development)

    include_directories(${PYTHON_INCLUDE_DIRS})

    add_subdirectory(external/pybind11)
    
    pybind11_add_module(${PROJECT_NAME} ${maiacore_src} ${python_wrapper_src})
    
    target_include_directories(${PROJECT_NAME} PUBLIC include)
    target_include_directories(${PROJECT_NAME} PUBLIC external/pugi/include)
    target_include_directories(${PROJECT_NAME} PUBLIC external)
    target_include_directories(${PROJECT_NAME} PUBLIC external/cherno)

    add_subdirectory(external/zip)
    add_subdirectory(external/sqlitecpp)

    if(WIN32)
        if(MSYS OR MINGW)
            message(STATUS "===> Selected Windows Compiler: MSYS or MINGW")
            # https://cython.readthedocs.io/en/latest/src/tutorial/appendix.html
            target_link_libraries(${PROJECT_NAME} PRIVATE zip -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive SQLiteCpp sqlite3)
        else()
            message(STATUS "===> Selected Windows Compiler: MSVC")
            # MSVC: Talvez nao precise de todas essas flags necessarias para o MSVC. Substituir a pthread pela winpthread? Remover o dl?
            target_link_libraries(${PROJECT_NAME} PRIVATE zip SQLiteCpp sqlite3 pthread dl)
        endif()
    else()
        # Linux e MacOSX
        target_link_libraries(${PROJECT_NAME} PRIVATE zip SQLiteCpp sqlite3 pthread dl)
    endif()
endif()
