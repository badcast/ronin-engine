cmake_minimum_required(VERSION 3.11)
#SET PROJECT NAME, VERSION
project(RoninEngineLibrary VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS YES)

if(CMAKE_SIZEOF_VOID_P STREQUAL "4")
    set(ARCHITECTURE "x86")
elseif(CMAKE_SIZEOF_VOID_P STREQUAL "8")
    set(ARCHITECTURE "x64")
endif()

message(STATUS "Architecture select ${ARCHITECTURE}")

set(RONINENGINE_VERSION_MIN 1.0)
set(RUNTIME_OUTPUT_NAME "across_rgl")


include_directories("${PROJECT_SOURCE_DIR}/include/ronin/")

set(RGE_LIBS "")

#compilling sources
add_subdirectory(source)

message(WARNING ${RGE_LIBS})

add_library(RoninEngineLibrary
    SHARED
    $<TARGET_OBJECTS:core>
    $<TARGET_OBJECTS:art_intel>
    $<TARGET_OBJECTS:graphics>
    $<TARGET_OBJECTS:input_output>
    $<TARGET_OBJECTS:resources>
    $<TARGET_OBJECTS:simulation>
    $<TARGET_OBJECTS:user_interface>
)

target_compile_definitions(RoninEngineLibrary PRIVATE -DDLLEXPORTING)

set_target_properties(RoninEngineLibrary PROPERTIES
    OUTPUT_NAME ${RUNTIME_OUTPUT_NAME})

target_sources(RoninEngineLibrary PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/source/lib.cpp")

#include to #include <ronin/...>
target_include_directories(RoninEngineLibrary INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")

