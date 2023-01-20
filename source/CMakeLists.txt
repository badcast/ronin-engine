
#generate

set(RGE_LIBS "")
set(sources "")

function(libdef libname path)
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${path}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/${path}/*.h")
    add_library(${libname}
        OBJECT ${SRC})
    list(APPEND RGE_LIBS "${libname}")

    target_compile_options(${libname}
        PRIVATE -fPIC)

    target_compile_definitions(${libname}
        PRIVATE
        -DRONIN_COMPILLING)
    set(sources "${sources}" "${SRC}")
endfunction()

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

file(GLOB_RECURSE RONIN_ENGINE_HEADERS
    "${PROJECT_SOURCE_DIR}/include/*")

set(RONIN_ENGINE_HEADERS ${RONIN_ENGINE_HEADERS} PARENT_SCOPE)

#RoninEngine include files
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)


#core sources
libdef(core "core")

#simulation sources
if(RGSIMULATION)
    libdef(simulation "simulation")
endif()
#res sources
if(RGRESOURCES)
    libdef(resources "res")
endif()
#io sources
if(RGIO)
    libdef(input_output "io")
endif()
#graphics sources
if(RGGRAPHICS)
    libdef(graphics "graphics")
endif()
#ui sources
if(RGGUI)
    libdef(user_interface "ui")
endif()
#ai sources
if(RGAI)
    libdef(art_intel "ai")
endif()
