cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(RONINENGINE_VERSION_MIN 1.0.0)

# SET PROJECT NAME, VERSION
project(RoninEngine VERSION ${RONINENGINE_VERSION_MIN})

set(RUNTIME_OUTPUT_NAME "roninengine")

option(BUILD_SHARED_LIBS "Build a shared lib ?" ON)
option(RGIO "Define Input/Output system" ON)
option(RGGRAPHICS "Define Graphics, Sprite, Textures system" ON)
option(RGRESOURCES "Define resources control" ON)
option(RGSIMULATION "Define level render" ON)
option(RGGUI "Define User Interface Control system" ON)
option(RGAI "Define Navigation and AI system" ON)
option(RGAUDIO "Define Audio Source and clips, based on SDL_Mixer" ON)

if(CMAKE_SIZEOF_VOID_P STREQUAL "4")
  set(ARCHITECTURE "x86")
elseif(CMAKE_SIZEOF_VOID_P STREQUAL "8")
  set(ARCHITECTURE "x64")
endif()

message(STATUS "Architecture select ${ARCHITECTURE}")

# ENVIRONMENT

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# RONIN ENGINE SOURCES
add_subdirectory(src)

add_library(RoninEngine ${ronin_targets} ${RONIN_ENGINE_HEADERS})

set_target_properties(RoninEngine PROPERTIES OUTPUT_NAME ${RUNTIME_OUTPUT_NAME})

target_compile_options(RoninEngine PRIVATE cxx_std_17)

# DEPENDENCIES

find_packages()

# INSTALLS

if(${RONIN_NO_CONFIG})
  target_include_directories(RoninEngine
                             INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
else()
  add_subdirectory(config)
endif()
