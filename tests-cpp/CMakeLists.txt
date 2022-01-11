cmake_minimum_required(VERSION 3.10)

project("cpp-tests" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -g -O0")

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

target_include_directories(${PROJECT_NAME} PUBLIC ../include)
target_include_directories(${PROJECT_NAME} PUBLIC ../external/pugi/include)
target_include_directories(${PROJECT_NAME} PUBLIC ../external)

if(WIN32)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Windows/static/debug)
elseif(APPLE)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Darwin/static/debug)
elseif(UNIX AND NOT APPLE)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Linux/static/debug)
else()
    message(FATAL_ERROR "Unable to detect the OS")
endif()

include_directories(${PROJECT_NAME} PUBLIC ../external/zip/src)
target_link_directories(${PROJECT_NAME} PUBLIC ../build/Windows/static/debug/external/zip)

target_link_libraries(${PROJECT_NAME} PUBLIC maialib zip gtest)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})




