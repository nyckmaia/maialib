cmake_minimum_required(VERSION 3.0)

project("cpp-tests" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

enable_testing()

# ===== GOOGLE TESTS ===== #
add_subdirectory(googletest)

add_executable(${PROJECT_NAME} 
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/helpers-test.cpp
    ${PROJECT_SOURCE_DIR}/src/note-test.cpp
    ${PROJECT_SOURCE_DIR}/src/chord-test.cpp
    ${PROJECT_SOURCE_DIR}/src/interval-test.cpp
    ${PROJECT_SOURCE_DIR}/src/nyck-test.cpp
    ${PROJECT_SOURCE_DIR}/src/adolfo-test.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ../maiacore/include)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ../maiacore/include/external)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ../maiacore/include/external/pugi)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ../maiacore/external/sqlitecpp/include)

include_directories(${PROJECT_NAME} PUBLIC ../maiacore/external/sqlitecpp/sqlite3)
include_directories(${PROJECT_NAME} PUBLIC ../maiacore/external/sqlitecpp/src)

if(WIN32)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Windows/static/Debug)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Windows/static/Debug/maiacore/external/sqlitecpp)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Windows/static/Debug/maiacore/external/sqlitecpp/sqlite3)

    target_link_libraries(${PROJECT_NAME} PUBLIC maiacore gtest SQLiteCpp sqlite3)
elseif(APPLE)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Darwin/static/Debug)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Darwin/static/Debug/maiacore/external/sqlitecpp)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Darwin/static/Debug/maiacore/external/sqlitecpp/sqlite3)

    target_link_libraries(${PROJECT_NAME} PUBLIC maiacore gtest SQLiteCpp sqlite3 dl)
elseif(UNIX AND NOT APPLE)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Linux/static/Debug)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Linux/static/Debug/maiacore/external/sqlitecpp)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Linux/static/Debug/maiacore/external/sqlitecpp/sqlite3)

    target_link_libraries(${PROJECT_NAME} PUBLIC maiacore gtest SQLiteCpp sqlite3 dl)
else()
    message(FATAL_ERROR "Unable to detect the OS")
endif()

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})




