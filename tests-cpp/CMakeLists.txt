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

include(FetchContent)
FetchContent_Declare(
  cpptrace
  GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
  GIT_TAG        v0.6.2 # <HASH or TAG>
)
FetchContent_MakeAvailable(cpptrace)

if(WIN32)
    set(OS "Windows")
elseif(APPLE)
    set(OS "Darwin")
elseif(UNIX AND NOT APPLE)
    set(OS "Linux")
else()
    message(FATAL_ERROR "Unable to detect the OS")
endif()

set(MAIALIB_ROOT_DIR ..)
set(MAIACORE_ROOT_DIR ${MAIALIB_ROOT_DIR}/maiacore)
set(MAIACORE_INCLUDE_DIR ${MAIACORE_ROOT_DIR}/include)

set(MAIACORE_LIB_DIR ${MAIALIB_ROOT_DIR}/build/${OS}/static/Debug)
set(SQLITECPP_LIB_DIR ${MAIACORE_LIB_DIR}/maiacore/external/sqlitecpp)
set(SQLITE_LIB_DIR ${SQLITECPP_LIB_DIR}/sqlite3)

target_include_directories(${PROJECT_NAME} PUBLIC ${MAIACORE_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${MAIACORE_INCLUDE_DIR}/external)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${MAIACORE_INCLUDE_DIR}/external/pugi)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${MAIACORE_ROOT_DIR}/external/sqlitecpp/include)

target_link_directories(${PROJECT_NAME} PUBLIC ${MAIACORE_LIB_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC ${SQLITECPP_LIB_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC ${SQLITE_LIB_DIR})

set(LIBS 
    maiacore
    gtest
    SQLiteCpp
    sqlite3
    cpptrace::cpptrace
)

if(MSVC)
    set(CPP_COMPILER_TYPE "MSVC")
    add_definitions(-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE)
elseif(MSYS)
    set(CPP_COMPILER_TYPE "MSYS")
    list(APPEND LIBS gtest_main)
elseif(MINGW)
    set(CPP_COMPILER_TYPE "MINGW")
    list(APPEND LIBS gtest_main)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CPP_COMPILER_TYPE "CLANG")
else()
    message(FATAL "===> UNABLE TO FIND A WINDOWS COMPILER")
endif()

message(STATUS "===> Selected Compiler Type: " ${CPP_COMPILER_TYPE})

if(APPLE OR (UNIX AND NOT APPLE))
    list(APPEND LIBS dl)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
