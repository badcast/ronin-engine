#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{

    struct GCMemoryStick {
        int memoryType;
        void* memory;
    };

    extern int gc_write_memblock_runtime(GCMemoryStick** ms, const std::uint8_t& typeIndex, const std::size_t size);
    extern int gc_native_collect(const int freeID);

    constexpr std::uint8_t SDL_TYPE_MAX_INDEX = 31;
    constexpr std::uint8_t InvalidType = 0xff;

    // any type get indexer
    template <typename, std::uint8_t>
    struct type2index_low;

    template <typename T, std::uint8_t _Index>
    struct type2index_low {
        typedef T type;
        static constexpr std::uint8_t typeIndex = _Index;
    };

    template <typename>
    struct type2index;

    template <>
    struct type2index<void> : public type2index_low<void, 0> { // Any pointer
    };
    template <>
    struct type2index<SDL_Texture> : public type2index_low<SDL_Texture, 1> { // SDL Texture
    };
    template <>
    struct type2index<SDL_Surface> : public type2index_low<SDL_Surface, 2> { // SDL Surface
    };
    template <>
    struct type2index<Texture> : public type2index_low<Texture, 32> { // Ronin Texture
    };
    template <>
    struct type2index<Sprite> : public type2index_low<Sprite, 33> { // Ronin Sprite
    };

    /// Object type index
    template <typename T>
    struct type2index : public type2index_low<
                            std::enable_if_t<std::is_base_of_v<Object, T>, T>,
                            128> { // Ronin Object
    };

    // TODO: indexToType is not completed (impl)
    template <std::uint8_t, typename J = void>
    struct index2type {
    };

    template <typename T>
    struct is_sdl_type : public std::integral_constant<bool, type2index<T>::typeIndex <= SDL_TYPE_MAX_INDEX> {
    };
    template <typename T>
    struct is_invalid_type : public std::integral_constant<bool, type2index<T>::typeIndex == InvalidType> {
    };
} // namespace RoninEngine::Runtime
