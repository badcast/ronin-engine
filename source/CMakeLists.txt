function(find_packages)
    if(LINUX)
        find_package(SDL2 REQUIRED)
        find_package(SDL2_image REQUIRED)
        find_package(SDL2_mixer REQUIRED)
        find_package(SDL2_ttf REQUIRED)
    elseif(WIN32)
        #set(CMAKE_GNUtoMS ON)

        #set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
        set(LIBS_DIR "${PROJECT_SOURCE_DIR}/third-party/SDL2/")

        #next use

        if(NOT EXISTS "${LIBS_DIR}")
            message(FATAL_ERROR "directory-libs not found, error binding SDL2")
        endif()

        set(CMAKE_PREFIX_PATH ${LIBS_DIR})

        find_package(SDL2 REQUIRED PATHS "${LIBS_DIR}" NO_DEFAULT_PATH)
        find_package(SDL2_image REQUIRED PATHS "${LIBS_DIR}" NO_DEFAULT_PATH)
        find_package(SDL2_mixer REQUIRED PATHS "${LIBS_DIR}" NO_DEFAULT_PATH)
        find_package(SDL2_ttf REQUIRED PATHS "${LIBS_DIR}" NO_DEFAULT_PATH)
        find_package(SDL2_gfx REQUIRED PATHS "${LIBS_DIR}" NO_DEFAULT_PATH)
    endif()
endfunction()

set(RGE_LIBS "")
set(sources "")

#region: excluding and ordering (internal,external) headers
file(GLOB_RECURSE RONIN_ENGINE_HEADERS
    "${PROJECT_SOURCE_DIR}/include/*")
set(RONIN_ENGINE_HEADERS "${RONIN_ENGINE_HEADERS}" PARENT_SCOPE)

#region: excluding and ordering (internal,external) headers
#file(GLOB_RECURSE RGE_HEADERS
#    "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

#set(EXCLUDE_OF
#    "begin.h"
#)

#message(STATUS Excluding:)
#foreach(F IN LISTS RONIN_ENGINE_HEADERS)
#    if("${F}" MATCHES "${EXCLUDE_OF}$")
#        message(STATUS "${F} excluded")
#    else()
#        list(APPEND RGE_HEADERS "${F}")
#    endif()
#endforeach()

#message(FATAL_ERROR "${RGE_HEADERS}")
#RoninEngine include files
#include_directories(
#external include
#    "${PROJECT_SOURCE_DIR}/include/ronin"
#internal include
#    "${CMAKE_CURRENT_SOURCE_DIR}/include"
#

function(libdef libname path)
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${path}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/${path}/*.h")

    add_library(${libname}
        OBJECT ${SRC})

    target_compile_options(${libname}
        PRIVATE -fPIC)

    target_compile_definitions(${libname}
        PRIVATE -DRONIN_COMPILLING)

    target_include_directories(${libname} PRIVATE
        "${CMAKE_SOURCE_DIR}/include/internal/ronin"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")


    list(APPEND RGE_LIBS "${libname}")
    list(APPEND sources "${SRC}")
endfunction()

#core sources
libdef(ronin "core")

set(ronin_targets
    $<TARGET_OBJECTS:ronin> PARENT_SCOPE)

#simulation sources
if(RGSIMULATION)
    libdef(ronin_simulation "simulation")
    list(APPEND ronin_targets $<TARGET_OBJECTS:ronin_simulation>)
endif()
#res sources
if(RGRESOURCES)
    libdef(ronin_resources "res")
    list(APPEND ronin_targets $<TARGET_OBJECTS:ronin_resources>)
endif()
#io sources
if(RGIO)
    libdef(ronin_input_output "io")
    list(APPEND ronin_targets $<TARGET_OBJECTS:ronin_input_output>)
endif()
#graphics sources
if(RGGRAPHICS)
    libdef(ronin_graphics "graphics")
    list(APPEND ronin_targets $<TARGET_OBJECTS:ronin_graphics>)
endif()
#gui sources
if(RGGUI)
    libdef(ronin_user_interface "ui")
    list(APPEND ronin_targets $<TARGET_OBJECTS:ronin_user_interface>)
endif()
#ai sources
if(RGAI)
    libdef(ronin_art_intel "ai")
    list(APPEND ronin_targets $<TARGET_OBJECTS:ronin_art_intel>)
endif()
