function(find_ronin_packages)
  find_package(PkgConfig REQUIRED)

  set(LINK_INCL)
  set(LINK_LIBS)

  pkg_check_modules(SDL2 REQUIRED sdl2)

  if(RGAUDIO)
    pkg_check_modules(SDL2_mixer REQUIRED SDL2_mixer)

    list(APPEND LINK_INCL ${SDL2_mixer_INCLUDE_DIRS})
    list(APPEND LINK_LIBS ${SDL2_mixer_LIBRARIES})
  endif()

  if(RGGRAPHICS)
    pkg_check_modules(SDL2_image REQUIRED SDL2_image)
    pkg_check_modules(SDL2_ttf REQUIRED SDL2_ttf)
    pkg_check_modules(SDL2_gfx REQUIRED SDL2_gfx)

    list(APPEND LINK_INCL ${SDL2_mixer_INCLUDE_DIRS} ${SDL2_image_INCLUDE_DIRS}
         ${SDL2_ttf_INCLUDE_DIRS} ${SDL2_gfx_INCLUDE_DIRS})
    list(APPEND LINK_LIBS ${SDL2_gfx_LIBRARIES} ${SDL2_image_LIBRARIES}
         ${SDL2_ttf_LIBRARIES})
  endif()

  pkg_check_modules(LibraryBrainMap brainmap)

  find_package(OpenMP)

  target_include_directories(RoninEngine PRIVATE ${SDL2_INCLUDE_DIRS}
                                                 ${LINK_INCL})

  target_link_libraries(RoninEngine PRIVATE ${SDL2_LIBRARIES} ${LINK_LIBS})

  target_compile_definitions(RoninEngine PRIVATE RONIN_COMPILLING)

endfunction()

set(RGE_LIBS "")
set(RGE_LINKED_NAMES "")
set(sources "")

# region: excluding and ordering (internal,external) headers
file(GLOB_RECURSE RONIN_ENGINE_HEADERS "${PROJECT_SOURCE_DIR}/include/*")

set(RONIN_ENGINE_HEADERS
    "${RONIN_ENGINE_HEADERS}"
    PARENT_SCOPE)

function(libdef libname path)
  set(path "${CMAKE_CURRENT_SOURCE_DIR}/${path}")
  file(GLOB_RECURSE SRC "${path}/*.cpp" "${path}/*.hpp" "${path}/*.h")

  if("${SRC}" STREQUAL "")
    return()
  endif()

  add_library(${libname} OBJECT ${SRC})

  target_include_directories(
    ${libname} PRIVATE $<BUILD_INTERFACE:${RONIN_INCLUDE_DIRS}/ronin>
                       ${CMAKE_CURRENT_SOURCE_DIR}/include)

  target_compile_features(${libname} PUBLIC cxx_std_17)

  if(NOT MSVC)
    target_compile_options(${libname} PRIVATE -fPIC)
  endif()

  target_compile_definitions(${libname} PRIVATE RONIN_COMPILLING)

  set(ronin_targets
      "${ronin_targets}" "$<TARGET_OBJECTS:${libname}>"
      PARENT_SCOPE)
  set(RGE_LIBS
      "${RGE_LIBS}" "${libname}"
      PARENT_SCOPE)
  list(APPEND RGE_LINKED_NAMES "${RGE_LINKED_NAMES}" PARENT_SCOPE)
endfunction()

# core sources
libdef(ronin "core")

file(GLOB_RECURSE SRC_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

target_sources(ronin PRIVATE ${SRC_INCLUDE})

# 3rdParty sources
libdef(ronin_3rdparty "3rdparty")
# simulation sources
if(RGSIMULATION)
  libdef(ronin_sim "sim")
endif()
# res sources
if(RGRESOURCES)
  libdef(ronin_res "res")
endif()
# io sources
if(RGIO)
  libdef(ronin_ios "io")
endif()
# graphics sources
if(RGGRAPHICS)
  libdef(ronin_gfx "gfx")
endif()
# gui sources
if(RGGUI)
  libdef(ronin_uif "ui")
endif()
# ai sources
if(RGAI)
  libdef(ronin_aif "ai")
  if(${LibraryBrainMap_FOUND})
    target_include_directories(ronin_aif
                               PRIVATE ${LibraryBrainMap_INCLUDE_DIRS})
    target_compile_definitions(ronin_aif PRIVATE OVERRIDE_LIBBRAINMAP)
  endif()
endif()
# audio sources
if(RGAUDIO)
  libdef(ronin_aud "audio")
endif()

set(ronin_targets
    "${ronin_targets}"
    PARENT_SCOPE)

target_compile_definitions(ronin
                           PRIVATE RGE_LINKED_TARGETS="${RGE_LINKED_NAMES}")
