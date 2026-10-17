cmake_minimum_required(VERSION 3.10)

find_package(PkgConfig QUIET)
if(NOT MSVC AND PkgConfig_FOUND)
  pkg_check_modules(SDL2 REQUIRED sdl2)
else()
  find_package(SDL2 REQUIRED)
endif()

set(TEST_TARGETS
    test_engine
    test_world_switch_reload
    test_bugfixes
    test_rendering
)

foreach(test_name ${TEST_TARGETS})
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE RoninEngine ${SDL2_LIBRARIES})
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/include ${SDL2_INCLUDE_DIRS})
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
