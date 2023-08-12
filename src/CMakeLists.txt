function(find_packages)
  find_package(PkgConfig REQUIRED)

  pkg_check_modules(SDL2 REQUIRED sdl2)
  pkg_check_modules(SDL2_image REQUIRED SDL2_image)
  pkg_check_modules(SDL2_mixer REQUIRED SDL2_mixer)
  pkg_check_modules(SDL2_ttf REQUIRED SDL2_ttf)
  pkg_check_modules(SDL2_gfx REQUIRED SDL2_gfx)
  pkg_check_modules(AcrossLibrary libacross)

  find_package(OpenMP)

  target_link_libraries(
    RoninEngine
    PRIVATE ${SDL2_LIBRARIES} ${SDL2_image_LIBRARIES} ${SDL2_mixer_LIBRARIES}
            ${SDL2_ttf_LIBRARIES} ${SDL2_gfx_LIBRARIES})

  target_include_directories(
    RoninEngine
    PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_image_INCLUDE_DIRS}
            ${SDL2_mixer_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS}
            ${SDL2_gfx_INCLUDE_DIRS})

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

  target_compile_options(${libname} PRIVATE -fPIC)

  target_compile_definitions(${libname} PRIVATE RONIN_COMPILLING)

  target_include_directories(
    ${libname}
    PRIVATE "${CMAKE_SOURCE_DIR}/include/ronin"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

  set(ronin_targets
      "${ronin_targets}" "$<TARGET_OBJECTS:${libname}>"
      PARENT_SCOPE)
  set(RGE_LIBS
      "${RGE_LIBS}" "${libname}"
      PARENT_SCOPE)
  set(sources
      ${sources} "${SRC}"
      PARENT_SCOPE)
  set(RGE_LINKED_NAMES
      "${libname} ${RGE_LINKED_NAMES}"
      PARENT_SCOPE)
endfunction()

file(GLOB_RECURSE RONIN_INCLUDE_MAINH
     "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

# core sources
libdef(ronin "core")

target_sources(ronin PUBLIC ${RONIN_INCLUDE_MAINH})

# simulation sources
if(RGSIMULATION)
  libdef(ronin_sim "simulation")
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
  if(${AcrossLibrary_FOUND})
    target_include_directories(ronin_aif PRIVATE ${AcrossLibrary_INCLUDE_DIRS})
    target_compile_definitions(ronin_aif PRIVATE OVERRIDE_LIBACROSS)
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
