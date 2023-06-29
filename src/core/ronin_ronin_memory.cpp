#include "ronin.h"

namespace RoninEngine::Runtime
{
    namespace RoninMemory
    {
        using namespace RoninEngine;

#if TEST_MALLOC
    std::list<void*> allocated_leaker;
#endif
        std::uint64_t __ronin_allocated = 0;
        void* ronin_memory_alloc(std::size_t size)
        {
            void* mem = std::malloc(size);
            if (mem == nullptr) {
                RoninSimulator::fail_oom_kill();
            }
            // Set up controls memory pointer. Set zero.
            memset(mem, 0, size);
            ++__ronin_allocated;
#if TEST_MALLOC
            allocated_leaker.push_back(mem);
#endif
            return mem;
        }

        void ronin_memory_free(void* memory)
        {
            std::free(memory);
            if (__ronin_allocated-- == 0)
                RoninSimulator::fail("invalid ronin_memory_free()");
#if TEST_MALLOC
            allocated_leaker.remove(memory);
#endif
        }

        std::uint64_t total_allocated() { return __ronin_allocated; }

    }
}
