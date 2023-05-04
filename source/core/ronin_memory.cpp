#include "ronin_memory.h"

namespace RoninEngine::Runtime
{
    namespace RoninMemory
    {
        using namespace RoninEngine;
        using namespace RoninEngine::Runtime;

        std::uint64_t __ronin_allocated = 0;


        void* malloc(std::size_t size)
        {
            void* mem = std::malloc(size);
            ++__ronin_allocated;
            return mem;
        }

        void mfree(void* memory)
        {
            std::free(memory);
            if (--__ronin_allocated < 0)
                Application::fail_OutOfMemory();
        }

        std::uint64_t total_allocated() { return __ronin_allocated; }

    }
}
