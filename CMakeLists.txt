cmake_minimum_required(VERSION 3.0)
#SET PROJECT NAME, VERSION
project(RoninEngine LANGUAGES CXX VERSION 1.0)

set(RONINENGINE_VERSION_MIN 1.0)
set(RUNTIME_OUTPUT_NAME "ronin.${RONINENGINE_VERSION_MIN}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_subdirectory(src/ronin) #get sources and linking from the "Sources" directory

add_library(RoninEngine SHARED ${RONINE_ENGINE_HEADERS} ${RONIN_ENGINE_SOURCES})
set_target_properties(RoninEngine PROPERTIES
    OUTPUT_NAME ${RUNTIME_OUTPUT_NAME})


#include to #include <ronin/...>
target_include_directories(RoninEngine INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")

#link SDL2
target_link_libraries(RoninEngine SDL2::SDL2 SDL2_mixer SDL2_ttf SDL2_image SDL2_gfx)
