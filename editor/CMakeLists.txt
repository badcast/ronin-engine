cmake_minimum_required(VERSION 3.16)

# Parse version from root VERSION file automatically
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../VERSION" RONIN_EDITOR_VERSION_RAW)
string(STRIP "${RONIN_EDITOR_VERSION_RAW}" RONIN_EDITOR_VERSION_STRING)
string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" RONIN_EDITOR_VERSION_SEMANTIC "${RONIN_EDITOR_VERSION_STRING}")

project(RoninEditor VERSION ${RONIN_EDITOR_VERSION_SEMANTIC} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Building Ronin Editor (Dear ImGui Docking) version: ${RONIN_EDITOR_VERSION_STRING}")

find_package(PkgConfig QUIET)
if(NOT MSVC AND PkgConfig_FOUND)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_MIXER QUIET SDL2_mixer)
    pkg_check_modules(SDL2_IMAGE QUIET SDL2_image)
else()
    find_package(SDL2 REQUIRED)
endif()

# Dear ImGui sources (docking branch)
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

set(RONIN_EDITOR_SOURCES
    src/main.cpp
    src/editor_app.cpp
    src/project_manager.cpp
    src/project_hub.cpp
    src/editor_state.cpp
    src/runtime_runner.cpp
    src/scene_viewport.cpp
    src/hierarchy_panel.cpp
    src/inspector_panel.cpp
    src/asset_browser.cpp
    src/asset_finder.cpp
    src/console_panel.cpp
    src/terrain_editor.cpp
    src/sprite_editor.cpp
    src/editor_icons.cpp
    ${IMGUI_SOURCES}
)

add_executable(RoninEditor ${RONIN_EDITOR_SOURCES})

target_include_directories(RoninEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
)

target_compile_definitions(RoninEditor PRIVATE
    RONIN_EDITOR_VERSION_STRING="${RONIN_EDITOR_VERSION_STRING}"
    RONIN_EDITOR_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    RONIN_EDITOR_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    RONIN_EDITOR_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

target_link_libraries(RoninEditor PRIVATE
    RoninEngine
    ${SDL2_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    SDL2_mixer
    SDL2_image
)
