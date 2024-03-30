#include "ronin.h"

namespace RoninEngine::Runtime
{
    namespace RoninMemory
    {
        using namespace RoninEngine;

#if TEST_MALLOC
        std::set<void *> allocated_leaker;
#endif
        std::uint64_t __ronin_allocated = 0;
        void *ronin_memory_alloc(std::size_t size)
        {
            void *mem = std::malloc(size);

            if(mem == nullptr)
                return nullptr;

            ++__ronin_allocated;

            // Set up controls memory pointer. Set to zero.
            memset(mem, 0, size);

#if TEST_MALLOC
            allocated_leaker.insert(mem);
#endif

            return mem;
        }

        void *ronin_memory_realloc(void *memory, std::size_t size)
        {
#if TEST_MALLOC
            allocated_leaker.erase(memory);
#endif
            void *prev = memory;
            memory = std::realloc(memory, size);
            if(prev == nullptr && memory != nullptr)
                ++__ronin_allocated;
            else if(prev != nullptr && size == 0)
                --__ronin_allocated;
#if TEST_MALLOC
            allocated_leaker.insert(memory);
#endif
            return memory;
        }

        void ronin_memory_free(void *memory)
        {
            std::free(memory);
            if(__ronin_allocated-- == 0)
                RoninSimulator::ShowMessageFail("invalid ronin_memory_free()");
#if TEST_MALLOC
            allocated_leaker.erase(memory);
#endif
        }

        std::uint64_t total_allocated()
        {
            return __ronin_allocated;
        }

    } // namespace RoninMemory
} // namespace RoninEngine::Runtime
