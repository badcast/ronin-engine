#pragma once

#include "ronin.h"

namespace RoninEngine::Runtime::RoninMemory
{
    void* malloc(std::size_t size);

    template <typename T>
    T* alloc()
    {
        extern std::uint64_t __ronin_allocated;
        T* mem = new T;
        ++__ronin_allocated;
        return mem;
    }

    template <typename T, typename... Args>
    T* alloc(Args&&... args)
    {
        extern std::uint64_t __ronin_allocated;
        T* mem = new T(std::forward<Args&&>(args)...);
        ++__ronin_allocated;
        return mem;
    }

    template <typename T>
    T* alloc_self(T*& self)
    {
        return self = alloc<T>();
    }

    template <typename T, typename... Args>
    T* alloc_self(T*& self, Args&&... args)
    {
        return self = alloc<T>(std::forward<Args&&>(args)...);
    }

    template <typename T>
    void free(T* memory)
    {
        extern std::uint64_t __ronin_allocated;
        delete memory;
        --__ronin_allocated;
    }

    void mfree(void* memory);

    std::uint64_t total_allocated();
}
