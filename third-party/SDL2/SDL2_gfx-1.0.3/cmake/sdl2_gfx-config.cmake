
cmake_minimum_required(VERSION 3.0)

set(SDL2_gfx_FOUND TRUE)

set(SDL2GFX_VENDORED TRUE)

set(SDL2GFX_HARFBUZZ TRUE)
set(SDL2GFX_FREETYPE TRUE)

if(CMAKE_SIZEOF_VOID_P STREQUAL "4" OR CMAKE_SIZEOF_VOID_P STREQUAL "8")
    set(_sdl_arch_subdir "x86") #every win32
else()
    unset(_sdl_arch_subdir)
    set(SDL2_gfx_FOUND FALSE)
    return()
endif()

set(_sdl2gfx_incdir       "${CMAKE_CURRENT_LIST_DIR}/../include")
set(_sdl2gfx_library      "${CMAKE_CURRENT_LIST_DIR}/../lib/${_sdl_arch_subdir}/SDL2_gfx.lib")
set(_sdl2gfx_dll          "${CMAKE_CURRENT_LIST_DIR}/../lib/${_sdl_arch_subdir}/SDL2_gfx.dll")

# All targets are created, even when some might not be requested though COMPONENTS.
# This is done for compatibility with CMake generated SDL2_image-target.cmake files.

if(NOT TARGET SDL2_gfx)
    add_library(SDL2_gfx SHARED IMPORTED)
    set_target_properties(SDL2_gfx
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${_sdl2gfx_incdir}"
            IMPORTED_IMPLIB "${_sdl2gfx_library}"
            IMPORTED_LOCATION "${_sdl2gfx_dll}"
            COMPATIBLE_INTERFACE_BOOL "SDL2_SHARED"
            INTERFACE_SDL2_SHARED "ON"
    )
endif()

unset(_sdl_arch_subdir)
unset(_sdl2gfx_incdir)
unset(_sdl2gfx_library)
unset(_sdl2gfx_dll)
