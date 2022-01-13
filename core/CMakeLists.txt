cmake_minimum_required(VERSION 3.0)

project("maialib" LANGUAGES CXX)

# Windows: If CMake errors -> Desable your antivirus!!

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -g -O0")

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

# Add all maialib source files
file(GLOB maialib_src
    "./src/*.cpp"
    "./src/python_wrapper/*.cpp"
    "./external/pugi/src/pugixml.cpp"
)

# Generate static library
if(STATIC_LIB)
    message("=====> GENERATING STATIC LIBRARY <=====")

    add_subdirectory(external/zip) 
    add_library(${PROJECT_NAME} STATIC ${maialib_src})

    target_include_directories(${PROJECT_NAME} PUBLIC include)
    target_include_directories(${PROJECT_NAME} PUBLIC external/pugi/include)
    target_include_directories(${PROJECT_NAME} PUBLIC external)

    target_link_libraries(${PROJECT_NAME} zip)
endif()

# Generate shared library
if(SHARED_LIB)
    message("=====> GENERATING SHARED LIBRARY <=====")
    add_subdirectory(external/zip)
    add_library(${PROJECT_NAME} SHARED ${maialib_src})

    target_include_directories(${PROJECT_NAME} PUBLIC include)
    target_include_directories(${PROJECT_NAME} PUBLIC external/pugi/include)
    target_include_directories(${PROJECT_NAME} PUBLIC external)

    target_link_libraries(${PROJECT_NAME} zip)
endif()

# Generate python module library
if(PYBIND_LIB)
    message("=====> GENERATING PYBIND LIBRARY <=====")

    add_definitions(-DPYBIND)
    
    find_package(Python COMPONENTS Interpreter Development)

    include_directories(${PYTHON_INCLUDE_DIRS})

    add_subdirectory(external/pybind11)
    
    pybind11_add_module(${PROJECT_NAME} ${maialib_src})
    
    target_include_directories(${PROJECT_NAME} PUBLIC include)
    target_include_directories(${PROJECT_NAME} PUBLIC external/pugi/include)
    target_include_directories(${PROJECT_NAME} PUBLIC external)

    add_subdirectory(external/zip)

    if(WIN32)
        if(MSYS OR MINGW)
            message(STATUS "===> Selected Windows Compiler: MSYS or MINGW")
            # https://cython.readthedocs.io/en/latest/src/tutorial/appendix.html
            target_link_libraries(${PROJECT_NAME} PRIVATE zip -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive)
        else()
            message(STATUS "===> Selected Windows Compiler: MSVC")
            # MSVC: Talvez nao precise de todas essas flags necessarias para o MSVC. Testar!
            target_link_libraries(${PROJECT_NAME} PRIVATE zip)
        endif()
    else()
        # Linux e MacOSX
        target_link_libraries(${PROJECT_NAME} PRIVATE zip)
    endif()
endif()
