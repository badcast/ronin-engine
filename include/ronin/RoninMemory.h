#pragma once

#include "begin.h"

namespace RoninEngine::Runtime::RoninMemory
{
    // main memory controller
    extern RONIN_API void* ronin_memory_alloc(std::size_t size);
    extern RONIN_API void ronin_memory_free(void* memory);

    template <typename T, typename... Args>
    constexpr inline T* _paste_oop_init(T* m, Args&&... args)
    {
        return new (m) T(std::forward<Args&&>(args)...);
    }

    template <typename T>
    constexpr inline T* _cut_oop_from(T* m)
    {
        m->~T();
        return m;
    }

    template <typename T, typename... Args>
    T* alloc(Args&&... args)
    {
        void* mem = ronin_memory_alloc(sizeof(T));
        return _paste_oop_init<T>(static_cast<T*>(mem), std::forward<Args&&>(args)...);
    }

    template <typename T>
    T* alloc_self(T*& self)
    {
        return self = alloc<T>();
    }

    template <typename T, typename... Args>
    T*& alloc_self(T*& self, Args&&... args)
    {
        return self = alloc<T>(std::forward<Args&&>(args)...);
    }

    template <typename T>
    void free(T* memory)
    {
        ronin_memory_free(_cut_oop_from(memory));
    }

    std::uint64_t total_allocated();
}
