#pragma once

#include "begin.h"

namespace RoninEngine::Runtime::RoninMemory
{
    // main memory controller
    RONIN_API void *ronin_memory_alloc(std::size_t size);
    RONIN_API void ronin_memory_free(void *memory);
    RONIN_API std::uint64_t total_allocated();

    template <typename T, typename... Args>
    constexpr inline T *_paste_oop_init(T *raw_memory, Args &&...args)
    {
        return new(raw_memory) T(std::forward<Args>(args)...);
    }

    template <typename T>
    constexpr inline T *_paste_oop_init(T *raw_memory)
    {
        return new(raw_memory) T();
    }

    template <typename T>
    constexpr inline T *_cut_oop_from(T *pointer)
    {
        pointer->~T();
        return pointer;
    }

    template <typename T, typename... Args>
    T *alloc(Args &&...args)
    {
        void *mem = ronin_memory_alloc(sizeof(T));
        return _paste_oop_init<T>(static_cast<T *>(mem), std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    T *alloc()
    {
        void *mem = ronin_memory_alloc(sizeof(T));
        return _paste_oop_init<T>(static_cast<T *>(mem));
    }

    template <typename T>
    T *alloc_self(T *&self)
    {
        return self = alloc<T>();
    }

    template <typename T, typename... Args>
    T *&alloc_self(T *&self, Args &&...args)
    {
        return self = alloc<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    void free(T *memory)
    {
        ronin_memory_free(_cut_oop_from(memory));
    }

} // namespace RoninEngine::Runtime::RoninMemory
