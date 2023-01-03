cmake_minimum_required(VERSION 3.4)
project(maiacore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
endif()

add_subdirectory(core/external/pybind11)

# Add all maiacore source files
file(GLOB maiacore_src
    "./core/src/*.cpp"
    "./core/external/pugi/src/pugixml.cpp"
)

# Add all python_wrapper source files
file(GLOB python_wrapper_src
    "./core/src/python_wrapper/*.cpp"
)

OPTION(PROFILING "Profile functions" OFF)

if(PROFILING)
  add_definitions(-DPROFILING)
endif()

add_definitions(-DMAIACORE_PYTHON_MODULE)

pybind11_add_module(${PROJECT_NAME} ${maiacore_src} ${python_wrapper_src})

target_include_directories(${PROJECT_NAME} PUBLIC core/include)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC core/external/pugi/include)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC core/external)
    
add_subdirectory(core/external/zip)
add_subdirectory(core/external/sqlitecpp)

# EXAMPLE_VERSION_INFO is defined by setup.py and passed into the C++ code as a
# define (VERSION_INFO) here.
target_compile_definitions(${PROJECT_NAME} PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})

message("=====> GENERATING PYBIND LIBRARY <=====")                
if(WIN32)
    if(MSYS OR MINGW)
        message(STATUS "===> Selected Windows Compiler: MSYS or MINGW")
        # https://cython.readthedocs.io/en/latest/src/tutorial/appendix.html
        target_link_libraries(${PROJECT_NAME} PRIVATE zip -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive SQLiteCpp sqlite3)
    else()
        message(STATUS "===> Selected Windows Compiler: MSVC")
        # MSVC: Talvez nao precise de todas essas flags necessarias para o MSVC. Substituir a pthread pela winpthread? Remover o dl?
        add_definitions(-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT") 
        target_link_libraries(${PROJECT_NAME} PRIVATE zip SQLiteCpp sqlite3)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
    endif()
else()
    # Linux e MacOSX
    target_link_libraries(${PROJECT_NAME} PRIVATE zip SQLiteCpp sqlite3 pthread dl)
endif()
