cmake_minimum_required(VERSION 3.25)

project("cpp-tests" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Definição para garantir a mesma configuração de runtime library
if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 --coverage")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
else()
    add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
    add_definitions(-DGTEST_HAS_PTHREAD=0)  # Desativa pthread para GoogleTest

    if(MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")

        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

        set(gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll" FORCE)
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -static")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static")
    endif()
endif()

enable_testing()

# ===== GOOGLE TESTS ===== #
add_subdirectory(googletest-1.14.0)

# Definir compilação estática para GoogleTest
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set_target_properties(gtest PROPERTIES COMPILE_FLAGS "-static")
    set_target_properties(gtest_main PROPERTIES COMPILE_FLAGS "-static")
    set_target_properties(gmock PROPERTIES COMPILE_FLAGS "-static")
    set_target_properties(gmock_main PROPERTIES COMPILE_FLAGS "-static")
endif()

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

include_directories(${PROJECT_NAME} SYSTEM PUBLIC ../maiacore/external/sqlitecpp/sqlite3)
include_directories(${PROJECT_NAME} SYSTEM PUBLIC ../maiacore/external/sqlitecpp/src)

if(WIN32)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Windows/static/Debug)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Windows/static/Debug/maiacore/external/sqlitecpp)
    target_link_directories(${PROJECT_NAME} PUBLIC ../build/Windows/static/Debug/maiacore/external/sqlitecpp/sqlite3)
    if(MSYS OR MINGW OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message(STATUS "===> Selected Windows Compiler: MSYS or MINGW or Clang")
        if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
            target_link_libraries(${PROJECT_NAME} PRIVATE gtest gtest_main SQLiteCpp sqlite3)
        else()
            target_link_libraries(${PROJECT_NAME} PRIVATE gtest gtest_main -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive SQLiteCpp sqlite3)
        endif()
    elseif(MSVC)
        message(STATUS "===> Selected Windows Compiler: MSVC")

        add_definitions(-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE)
        target_link_libraries(${PROJECT_NAME} PRIVATE SQLiteCpp sqlite3)
    else()
        message(FATAL "===> UNABLE TO FIND A WINDOWS COMPILER")
    endif()
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
