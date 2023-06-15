#include "ronin.h"

namespace RoninEngine::Runtime
{
    namespace RoninMemory
    {
        using namespace RoninEngine;

        std::uint64_t __ronin_allocated = 0;

        std::list<int*> __R;

        void* ronin_memory_alloc(std::size_t size)
        {
            void* mem = std::malloc(size);
            if (mem == nullptr) {
                Application::fail_oom_kill();
            }
            // Set up controls memory pointer. Set zero.
            memset(mem, 0, size);
            ++__ronin_allocated;
            __R.emplace_back((int*)mem);
            return mem;
        }

        void ronin_memory_free(void* memory)
        {
            std::free(memory);
            if (__ronin_allocated-- == 0)
                Application::fail("invalid ronin_memory_free()");
            __R.remove((int*)memory);
        }

        std::uint64_t total_allocated() { return __ronin_allocated; }

    }
}
