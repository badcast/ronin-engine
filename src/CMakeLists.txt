
set(RGE_LIBS "")
set(RGE_LINKED_NAMES "")
set(sources "")

function (custom_findpkg PKGNAME)

    if(NOT MSVC AND PkgConfig_FOUND)

        set(OPT_BASE_PKGNAME ${ARGV1})

        if(NOT DEFINED OPT_BASE_PKGNAME)
            set(OPT_BASE_PKGNAME ${PKGNAME})
        endif()

        pkg_check_modules(${PKGNAME} REQUIRED ${OPT_BASE_PKGNAME})

    elseif(MSVC)

        message(STATUS "Using Windows deps for MSVC compiller")

        set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/src/windows/deps")
        set(SDL2_DIR ${CMAKE_PREFIX_PATH}/SDL2-2.30.1)
        message(FATAL_ERROR ${CMAKE_PREFIX_PATH})

        find_package(${PKGNAME} REQUIRED NO_DEFAULT_PATH)

    else()

        find_package(${PKGNAME} REQUIRED)

    endif()

    list(APPEND LIST_INCLUDE_PKGS "${${PKGNAME}_INCLUDE_DIRS}")
    list(APPEND LIST_LIBS_PKGS "${${PKGNAME}_LIBRARIES}")

    # Set to parent SCOPE
    set(LIST_INCLUDE_PKGS ${LIST_INCLUDE_PKGS} PARENT_SCOPE)
    set(LIST_LIBS_PKGS ${LIST_LIBS_PKGS} PARENT_SCOPE)
endfunction(custom_findpkg)

function(custom_find_deps MainTarget)

  # LIST_INCLUDE_PKGS - There is All libs <*> from packages.
  set(LIST_INCLUDE_PKGS)
  # LIST_LIBS_PKGS - There is All include list <include> from packages.
  set(LIST_LIBS_PKGS)

  if(NOT TARGET ${MainTarget})
      Message(FATAL_ERROR "Require existence target argument")
  endif()


  # Fist PkgConfig for finder
  if(NOT MSVC)
    find_package(PkgConfig)
  else()

  endif()

  # find_package(OpenMP)

  custom_findpkg(SDL2 sdl2)

  if(RGAUDIO)
    custom_findpkg(SDL2_mixer)
  endif()

  if(RGGRAPHICS)
    custom_findpkg(SDL2_image)
    custom_findpkg(SDL2_ttf)
  endif()

  target_compile_definitions(${MainTarget} PRIVATE RONIN_COMPILLING=1)
  target_include_directories(${MainTarget} PRIVATE ${LIST_INCLUDE_PKGS})
  target_link_libraries(${MainTarget} PRIVATE ${LIST_LIBS_PKGS})
  target_compile_options(${MainTarget} PRIVATE -mrdrnd)

endfunction(custom_find_deps)

# region: excluding and ordering (internal,external) headers
file(GLOB_RECURSE RONIN_ENGINE_HEADERS "${PROJECT_SOURCE_DIR}/include/*")

set(RONIN_ENGINE_HEADERS
    "${RONIN_ENGINE_HEADERS}"
    PARENT_SCOPE)

function(custom_load_sources libname localPath)
  set(SRC_FULL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${localPath}")
  file(GLOB_RECURSE SRC "${SRC_FULL_PATH}/*.cpp" "${SRC_FULL_PATH}/*.c" "${SRC_FULL_PATH}/*.hpp" "${SRC_FULL_PATH}/*.h")

  if(SRC STREQUAL "")
      message(WARNING "${libname} on ${localPath} has empty siyurced")
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

endfunction(custom_load_sources)

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
  target_compile_options(RoninSimulations PRIVATE -mrdrnd)
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
