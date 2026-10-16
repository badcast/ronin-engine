#include "ronin.h"

#include <mutex>

namespace RoninEngine::Runtime
{
    namespace RoninMemory
    {
        using namespace RoninEngine;

#ifndef USE_SDL_MEM
#define USE_SDL_MEM 0
#endif

        static std::mutex mem_mutex;
        int __ronin_allocated = 0;

#if TEST_MALLOC
        std::set<void *> allocated_leaker;
#endif
        void *mem_alloc(std::size_t size)
        {
            void *memory;

#if USE_SDL_MEM
            memory = SDL_malloc(size);
#else
            memory = std::malloc(size);
#endif

            if(memory == nullptr)
                return nullptr;

            {
                std::lock_guard<std::mutex> lock(mem_mutex);
                ++__ronin_allocated;

#if TEST_MALLOC
                allocated_leaker.insert(memory);
#endif
            }

            // Set up controls memory pointer. Set to zero.
            memset(memory, 0, size);

            return memory;
        }

        void *mem_realloc(void *memory, std::size_t size)
        {
            std::lock_guard<std::mutex> lock(mem_mutex);
#if TEST_MALLOC
            allocated_leaker.erase(memory);
#endif
            void *prev = memory;

#if USE_SDL_MEM
            memory = SDL_realloc(memory, size);
#else
            memory = std::realloc(memory, size);
#endif
            if(prev == nullptr && memory != nullptr)
                ++__ronin_allocated;
            else if(prev != nullptr && size == 0)
                --__ronin_allocated;
#if TEST_MALLOC
            if(memory != nullptr)
                allocated_leaker.insert(memory);
#endif
            return memory;
        }

        void mem_free(void *memory)
        {
            if(memory == nullptr)
                return;

#if USE_SDL_MEM
            SDL_free(memory);
#else
            std::free(memory);
#endif
            std::lock_guard<std::mutex> lock(mem_mutex);
            if(__ronin_allocated-- <= 0)
            {
                __ronin_allocated = 0;
                RoninSimulator::ShowMessageFail("Fail on free memory");
            }
#if TEST_MALLOC
            allocated_leaker.erase(memory);
#endif
        }

        int mem_allocated()
        {
            std::lock_guard<std::mutex> lock(mem_mutex);
            return __ronin_allocated;
        }

    } // namespace RoninMemory
} // namespace RoninEngine::Runtime
