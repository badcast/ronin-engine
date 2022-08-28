cmake_minimum_required(VERSION 3.0)
#SET PROJECT NAME, VERSION
project(RoninEngine LANGUAGES CXX VERSION 1.0)

if(CMAKE_SIZEOF_VOID_P STREQUAL "4")
    set(ARCHITECTURE "x86")
elseif(CMAKE_SIZEOF_VOID_P STREQUAL "8")
    set(ARCHITECTURE "x64")
endif()

message(STATUS "Architecture select ${ARCHITECTURE}")

set(RONINENGINE_VERSION_MIN 1.0)
set(RUNTIME_OUTPUT_NAME "direct_rgl")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX)
	add_library(RoninEngineLib SHARED)
	
	find_package(SDL2 REQUIRED)
elseif(WIN32)
	set(SDL2_VERSION_WININTERNAL "2.0.22")
	add_library(RoninEngineLib STATIC)

        find_package(SDL2 QUIET)

	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

	if(NOT ${SDL2_FOUND})
		set(LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/SDL2")
		#next use 
		if(NOT EXISTS "${LIBS_DIR}")
			message(FATAL_ERROR "directory-libs not found, error binding SDL2")
		endif()	 

		#DO 
		set(CMAKE_PREFIX_PATH ${LIBS_DIR})
		
		target_include_directories(RoninEngineLib INTERFACE "${LIBS_DIR}/SDL2-${SDL2_VERSION_WININTERNAL}/include")
		message(STATUS "internal SDL2 - binded. Include use SDL2/SDL.h")
		
		set(STATIC_LIBS_SDL2PATH "${LIBS_DIR}/SDL2-${SDL2_VERSION_WININTERNAL}/lib/${ARCHITECTURE}")

		add_library(SDL2::SDL2 SHARED IMPORTED)

		set_target_properties(SDL2::SDL2 PROPERTIES
			IMPORTED_LOCATION "SDL2"
			IMPORTED_IMPLIB  "${STATIC_LIBS_SDL2PATH}/SDL2.lib" 
		)
	endif()
endif()

find_package(SDL2_mixer REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

target_link_libraries(RoninEngineLib SDL2::SDL2 SDL2_mixer SDL2_ttf SDL2_image SDL2_gfx)

set_target_properties(RoninEngineLib PROPERTIES
    OUTPUT_NAME ${RUNTIME_OUTPUT_NAME})

add_subdirectory(src/ronin) #get sources and linking from the "Sources" directory

#include to #include <ronin/...>
target_include_directories(RoninEngineLib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")

#ronin engine sources (cpp) only for RoninEngine Lib
target_sources(RoninEngineLib PRIVATE ${RONIN_ENGINE_HEADERS} ${RONIN_ENGINE_SOURCES})

