#include "ronin.h"

namespace RoninEngine::Runtime
{
    namespace RoninMemory
    {
        using namespace RoninEngine;
        using namespace RoninEngine::Runtime;

        std::uint64_t __ronin_allocated = 0;

        void* ronin_memory_alloc(std::size_t size)
        {
            void* mem = std::malloc(size);
            if (mem == nullptr) {
                Application::fail_oom_kill();
            }
            memset(mem, 0, size);
            ++__ronin_allocated;
            return mem;
        }

        void ronin_memory_free(void* memory)
        {
            std::free(memory);
            if (__ronin_allocated-- == 0)
                Application::fail("invalid ronin_memory_free()");
        }

        std::uint64_t total_allocated() { return __ronin_allocated; }

    }
}
