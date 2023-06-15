#include "ronin.h"

namespace RoninEngine::Runtime
{
    namespace RoninMemory
    {
        using namespace RoninEngine;

        std::uint64_t __ronin_allocated = 0;

#define TEST_MEMALLOC 0

#if TEST_MEMALLOC
        std::list<void*> allocated_leaker;
#endif
        void* ronin_memory_alloc(std::size_t size)
        {
            void* mem = std::malloc(size);
            if (mem == nullptr) {
                Application::fail_oom_kill();
            }
            // Set up controls memory pointer. Set zero.
            memset(mem, 0, size);
            ++__ronin_allocated;
#if TEST_MEMALLOC
            allocated_leaker.emplace_back(mem);
#endif
            return mem;
        }

        void ronin_memory_free(void* memory)
        {
            std::free(memory);
            if (__ronin_allocated-- == 0)
                Application::fail("invalid ronin_memory_free()");
#if TEST_MEMALLOC
            allocated_leaker.remove(memory);
#endif
        }

        std::uint64_t total_allocated() { return __ronin_allocated; }

    }
}
