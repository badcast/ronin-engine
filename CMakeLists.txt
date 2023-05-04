cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
# SET PROJECT NAME, VERSION
project(RoninEngineLibrary VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "BUILD a shared lib ?" ON)
option(RGIO "Define Input and Ouput system" ON)
option(RGGRAPHICS "Define Grahics, Sprite, Textures system" ON)
option(RGRESOURCES "Define resources control" ON)
option(RGSIMULATION "Define level render" ON)
option(RGGUI "Define User Control system" ON)
option(RGAI "Define Navigation and AI system" ON)

if(CMAKE_SIZEOF_VOID_P STREQUAL "4")
  set(ARCHITECTURE "x86")
elseif(CMAKE_SIZEOF_VOID_P STREQUAL "8")
  set(ARCHITECTURE "x64")
endif()

message(STATUS "Architecture select ${ARCHITECTURE}")

set(RONINENGINE_VERSION_MIN 1.0)
set(RUNTIME_OUTPUT_NAME "RoninEngine")

# compilling sources
add_subdirectory(source)

add_library(RoninEngineLibrary SHARED ${ronin_targets} ${RONIN_ENGINE_HEADERS})

set_target_properties(RoninEngineLibrary PROPERTIES OUTPUT_NAME
                                                    ${RUNTIME_OUTPUT_NAME})

# include to #include <ronin/...>
target_include_directories(RoninEngineLibrary
                           INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

find_packages()

target_link_libraries(
  RoninEngineLibrary PUBLIC SDL2::SDL2 SDL2_mixer::SDL2_mixer
                            SDL2_ttf::SDL2_ttf SDL2_gfx SDL2_image::SDL2_image)
