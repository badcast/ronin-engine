cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(RONINENGINE_VERSION_MIN 0.0.1)

# SET PROJECT NAME, VERSION
project(
  RoninEngine
  VERSION ${RONINENGINE_VERSION_MIN}
  LANGUAGES C CXX)

set(RUNTIME_OUTPUT_NAME "RoninEngine")

option(BUILD_SHARED_LIBS "Build a shared lib?" ON)
option(RGSIMULATION "Define level render" ON)
option(RGIO "Define Input/Output system" ON)
option(RGGRAPHICS "Define Graphics, Sprite, Textures system" ON)
option(RGRESOURCES "Define resources control" ON)
option(RGGUI "Define User Interface Control system" ON)
option(RGAI "Define Navigation and AI system" ON)
option(RGAUDIO "Define Audio Source and clips, derived from SDL_Mixer" ON)

if(CMAKE_SIZEOF_VOID_P STREQUAL "4")
  set(ARCHITECTURE "x86")
elseif(CMAKE_SIZEOF_VOID_P STREQUAL "8")
  set(ARCHITECTURE "x64")
endif()

message(STATUS "Architecture select ${ARCHITECTURE}")

if(NOT
   (RGIO
    AND RGGRAPHICS
    AND RGRESOURCES
    AND RGSIMULATION
    AND RGGUI
    AND RGAI
    AND RGAUDIO))
  message(
    FATAL_ERROR
      "One or more components are turned off. This is an unstable state. Please enable ALL components for proper operation."
  )
endif()

# ENVIRONMENT

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(RONIN_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

# RONIN ENGINE SOURCES

add_subdirectory(src)

add_library(RoninEngine ${ronin_targets} ${RONIN_ENGINE_HEADERS})

target_link_directories(RoninEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(RoninEngine PROPERTIES OUTPUT_NAME ${RUNTIME_OUTPUT_NAME})

# DEPENDENCIES

custom_find_deps(RoninEngine)

# INSTALLS

if(WIN32)
  # generate rc files
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/RoninEngine.rc.in
                 ${CMAKE_CURRENT_BINARY_DIR}/RoninEngine.rc @ONLY)

  add_custom_target(RoninWin32RC
                    SOURCES ${CMAKE_CURRENT_BINARY_DIR}/RoninEngine.rc)
  add_dependencies(RoninEngine RoninWin32RC)
  target_sources(RoninEngine PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/RoninEngine.rc)
endif()

if(${RONIN_NO_CONFIG})
  target_include_directories(
    RoninEngine PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>)
else()
  add_subdirectory(config)
endif()
