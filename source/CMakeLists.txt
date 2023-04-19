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

        find_package(SDL2          REQUIRED )
        find_package(SDL2_image    REQUIRED )
        find_package(SDL2_mixer    REQUIRED )
        find_package(SDL2_ttf      REQUIRED )
    endif()
endfunction()

set(RGE_LIBS "")
set(sources "")

#region: excluding and ordering (internal,external) headers
file(GLOB_RECURSE RONIN_ENGINE_HEADERS
    "${PROJECT_SOURCE_DIR}/include/*")

set(RONIN_ENGINE_HEADERS "${RONIN_ENGINE_HEADERS}" PARENT_SCOPE)

function(libdef libname path)
    set(path "${CMAKE_CURRENT_SOURCE_DIR}/${path}")
    file(GLOB SRC "${path}/*.cpp" "${path}/*.hpp" "${path}/*.h")

    if("${SRC}" STREQUAL "")
        return()
    endif()

    add_library(${libname}
        OBJECT ${SRC})

    target_compile_options(${libname}
        PRIVATE -fPIC)

    target_compile_definitions(${libname}
        PRIVATE -DRONIN_COMPILLING)

    target_include_directories(${libname} PRIVATE
        "${CMAKE_SOURCE_DIR}/include/ronin"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")

#    if(NOT "${libname}" STREQUAL "ronin")
#        target_link_libraries(ronin PRIVATE ${libname})
#    endif()

    set(ronin_targets "${ronin_targets}" "$<TARGET_OBJECTS:${libname}>" PARENT_SCOPE)
    set(RGE_LIBS "${RGE_LIBS}" "${libname}" PARENT_SCOPE)
    set(sources ${sources} "${SRC}" PARENT_SCOPE)
endfunction()

#core sources
libdef(ronin "core")

#simulation sources
if(RGSIMULATION)
    libdef(ronin_simulation "simulation")
endif()
#res sources
if(RGRESOURCES)
    libdef(ronin_resources "res")
endif()
#io sources
if(RGIO)
    libdef(ronin_input_output "io")
endif()
#graphics sources
if(RGGRAPHICS)
    libdef(ronin_graphics "graphics")
endif()
#gui sources
if(RGGUI)
    libdef(ronin_user_interface "ui")
endif()
#ai sources
if(RGAI)
    libdef(ronin_art_intel "ai")
endif()

 set(ronin_targets "${ronin_targets}"  PARENT_SCOPE)
