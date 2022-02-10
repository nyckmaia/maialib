cmake_minimum_required(VERSION 3.0)

project("cpp-tests" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O2")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

enable_testing()

# ===== GOOGLE TESTS ===== #
add_subdirectory(googletest)

add_executable(${PROJECT_NAME} 
    ./src/main.cpp
    ./src/helpers-test.cpp
    ./src/note-test.cpp
    ./src/chord-test.cpp
    ./src/nyck-test.cpp
    ./src/adolfo-test.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ../core/include)
target_include_directories(${PROJECT_NAME} PUBLIC ../core/external/pugi/include)
target_include_directories(${PROJECT_NAME} PUBLIC ../core/external)
target_include_directories(${PROJECT_NAME} PUBLIC ../core/external/sqlitecpp/include)

include_directories(${PROJECT_NAME} PUBLIC ../core/external/zip/src)
include_directories(${PROJECT_NAME} PUBLIC ../core/external/sqlitecpp/sqlite3)
include_directories(${PROJECT_NAME} PUBLIC ../core/external/sqlitecpp/src)

if(WIN32)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Windows/static/debug)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Windows/static/debug/external/zip)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Windows/static/debug/external/sqlitecpp)
elseif(APPLE)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Darwin/static/debug)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Darwin/static/debug/external/zip)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Darwin/static/debug/external/sqlitecpp)
elseif(UNIX AND NOT APPLE)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Linux/static/debug)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Linux/static/debug/external/zip)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Linux/static/debug/external/sqlitecpp)
else()
    message(FATAL_ERROR "Unable to detect the OS")
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC maiacore zip gtest SQLiteCpp sqlite3)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})




