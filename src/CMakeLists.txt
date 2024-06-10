function (custom_findpkg p1 p2)

    if(PkgConfig_FOUND)

        pkg_check_modules(${p1} REQUIRED ${p2})


    else()

    endif()




endfunction()

function(find_ronin_packages)

  set(LINK_INCL)
  set(LINK_LIBS)

  find_package(PkgConfig)

  custom_findpkg(SDL2 sdl2)

  if(RGAUDIO)
    custom_findpkg(SDL2_mixer SDL2_mixer)

    list(APPEND LINK_INCL ${SDL2_mixer_INCLUDE_DIRS})
    list(APPEND LINK_LIBS ${SDL2_mixer_LIBRARIES})
  endif()

  if(RGGRAPHICS)
    custom_findpkg(SDL2_image SDL2_image)
    custom_findpkg(SDL2_ttf SDL2_ttf)
    custom_findpkg(SDL2_gfx SDL2_gfx)

    list(APPEND LINK_INCL ${SDL2_mixer_INCLUDE_DIRS} ${SDL2_image_INCLUDE_DIRS}
         ${SDL2_ttf_INCLUDE_DIRS} ${SDL2_gfx_INCLUDE_DIRS})
    list(APPEND LINK_LIBS ${SDL2_gfx_LIBRARIES} ${SDL2_image_LIBRARIES}
         ${SDL2_ttf_LIBRARIES})
  endif()

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

function(custom_load_sources libname path)
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
custom_load_sources(RoninCore "core")

file(GLOB_RECURSE SRC_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

target_sources(RoninCore PRIVATE ${SRC_INCLUDE})

# 3rdParty sources
custom_load_sources(Ronin3rdParty "3rdparty")

# assign splash world


# simulation sources
if(RGSIMULATION)
  custom_load_sources(RoninSimulations "sim")
endif()
# res sources
if(RGRESOURCES)
  custom_load_sources(RoninResources "res")
endif()
# io sources
if(RGIO)
  custom_load_sources(RoninInputOutputs "io")
endif()
# graphics sources
if(RGGRAPHICS)
  custom_load_sources(RoninGraphics "gfx")
endif()
# gui sources
if(RGGUI)
  custom_load_sources(RoninUserInterface "ui")
endif()
# ai sources
if(RGAI)
  custom_load_sources(RoninArtInteleg "ai")
  if(${LibraryBrainMap_FOUND})
    target_include_directories(ronin_aif
                               PRIVATE ${LibraryBrainMap_INCLUDE_DIRS})
    target_compile_definitions(ronin_aif PRIVATE OVERRIDE_LIBBRAINMAP)
  endif()
endif()
# audio sources
if(RGAUDIO)
  custom_load_sources(RoninAudio "audio")
endif()

set(ronin_targets
    "${ronin_targets}"
    PARENT_SCOPE)

target_compile_definitions(RoninCore
                           PRIVATE RGE_LINKED_TARGETS="${RGE_LINKED_NAMES}")
