# Путь к заголовочным файлам SDL2_gfx
find_path(SDL2_GFX_INCLUDE_DIR SDL2_gfxPrimitives.h PATHS ${CMAKE_CURRENT_LIST_DIR}/../include)

#Support only x64
find_library(SDL2_GFX_LIBRARY SDL2_gfx PATHS ${CMAKE_CURRENT_LIST_DIR}/../lib/x64)

# Проверка найденных файлов
if (SDL2_GFX_INCLUDE_DIR AND SDL2_GFX_LIBRARY)
    set(SDL2_GFX_FOUND TRUE)
endif()

# Экспорт результатов поиска
if (SDL2_GFX_FOUND)
    if (NOT SDL2_GFX_FIND_QUIETLY)
        message(STATUS "Found SDL2_gfx: ${SDL2_GFX_LIBRARY}")
    endif()
else()
    if (NOT SDL2_GFX_FIND_QUIETLY)
        message(FATAL_ERROR "SDL2_gfx not found")
    endif()
endif()

# Установка переменных для использования в проекте
if (SDL2_GFX_FOUND)
    set(SDL2_GFX_INCLUDE_DIRS ${SDL2_gfx_INCLUDE_DIR})
    set(SDL2_GFX_LIBRARIES ${SDL2_gfx_LIBRARY})
endif()

mark_as_advanced(SDL2_GFX_INCLUDE_DIR SDL2_GFX_LIBRARY)
