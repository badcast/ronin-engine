cmake_minimum_required(VERSION 3.0)
#SET PROJECT NAME, VERSION
project(RoninEngineLibrary LANGUAGES CXX VERSION 1.0)

if(CMAKE_SIZEOF_VOID_P STREQUAL "4")
    set(ARCHITECTURE "x86")
elseif(CMAKE_SIZEOF_VOID_P STREQUAL "8")
    set(ARCHITECTURE "x64")
endif()

message(STATUS "Architecture select ${ARCHITECTURE}")

set(RONINENGINE_VERSION_MIN 1.0)
set(RUNTIME_OUTPUT_NAME "across_rgl")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX)
        add_library(RoninEngineLibrary SHARED)
elseif(WIN32)
        add_library(RoninEngineLibrary SHARED)

        #set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
        set(LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/SDL2/")
        #next use
        if(NOT EXISTS "${LIBS_DIR}")
                message(FATAL_ERROR "directory-libs not found, error binding SDL2")
        endif()
        set(CMAKE_PREFIX_PATH ${LIBS_DIR})
        find_package(SDL2_gfx REQUIRED)
endif()

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

target_link_libraries(RoninEngineLibrary SDL2::SDL2 SDL2_mixer::SDL2_mixer SDL2_ttf::SDL2_ttf SDL2_gfx SDL2_image::SDL2_image)

set_target_properties(RoninEngineLibrary PROPERTIES
    OUTPUT_NAME ${RUNTIME_OUTPUT_NAME})

#get sources and linking from the "Sources" directory
file(GLOB RONIN_ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

file(GLOB RONIN_ENGINE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ronin/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ronin/*.h")

set(RONIN_ENGINE_SOURCES ${RONIN_ENGINE_SOURCES} PARENT_SCOPE)
set(RONIN_ENGINE_HEADERS ${RONIN_ENGINE_HEADERS} PARENT_SCOPE)
set(RONIN_ENGINE_INCLUDE_DIRS ${RONIN_ENGINE_HEADERS} PARENT_SCOPE)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/ronin/")

#include to #include <ronin/...>
target_include_directories(RoninEngineLibrary INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")

#ronin engine sources (cpp) only for RoninEngine Lib
target_sources(RoninEngineLibrary PRIVATE ${RONIN_ENGINE_HEADERS} ${RONIN_ENGINE_SOURCES})
