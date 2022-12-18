
#generate

set(SOURCE_MASK "*.cpp")
set(RGE_LIBS "")
set(sources "")
function(libdef libname path)
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${path}/${SOURCE_MASK}")
    add_library(${libname}
        OBJECT ${SRC})
    list(APPEND RGE_LIBS "${libname}")
    target_compile_options(${libname}
        PRIVATE -fPIC)
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


file(GLOB RONIN_ENGINE_HEADERS
    "${PROJECT_SOURCE_DIR}/include/ronin/*.hpp"
    "${PROJECT_SOURCE_DIR}/include/ronin/*.h")

find_packages()

#core sources
libdef(core "core")

#simulation sources
libdef(simulation "simulation")

#res sources
libdef(resources "res")

#io sources
libdef(input_output "io")

#graphics sources
libdef(graphics "graphics")

#ui sources
libdef(user_interface "ui")

#ai sources
libdef(art_intel "ai")

target_link_libraries(core SDL2::SDL2 SDL2_mixer::SDL2_mixer SDL2_ttf::SDL2_ttf SDL2_gfx SDL2_image::SDL2_image)

set(RONIN_ENGINE_HEADERS ${RONIN_ENGINE_HEADERS} PARENT_SCOPE)
