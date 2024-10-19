#include <SDL2/SDL.h>

#include "SystemInformation.h"

#ifdef __linux__

#define USE_PROCFS 1

#if USE_PROCFS
#include <cstdio>
#include <fstream>
#endif

#endif

#if WIN32
typedef struct _PROCESS_MEMORY_COUNTERS_EX
{
    unsigned long cb;
    unsigned long PageFaultCount;
    size_t PeakWorkingSetSize;
    size_t WorkingSetSize;
    size_t QuotaPeakPagedPoolUsage;
    size_t QuotaPagedPoolUsage;
    size_t QuotaPeakNonPagedPoolUsage;
    size_t QuotaNonPagedPoolUsage;
    size_t PagefileUsage;
    size_t PeakPagefileUsage;
    size_t PrivateUsage;
} PROCESS_MEMORY_COUNTERS_EX;
typedef PROCESS_MEMORY_COUNTERS_EX *PPROCESS_MEMORY_COUNTERS_EX;

extern "C"
{
    __declspec(dllimport) void *__stdcall GetCurrentProcess(void);
    __declspec(dllimport) int __stdcall K32GetProcessMemoryInfo(void *Process, PROCESS_MEMORY_COUNTERS_EX *ppsmemCounters, unsigned long cb);
}

#elif __unix__
#if USE_PROCFS

#include <sys/sysinfo.h>
#include <sys/types.h>

int parseLine(char *line)
{
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char *p = line;
    while(*(p) < '0' || *(p) > '9')
        p++;
    line[i - 3] = '\0';
    i = atoi(p);
    return i;
}

size_t get_memory_used()
{
    char line[64];
    size_t result;
    std::ifstream file("/proc/self/status");
    if(file.is_open())
    {
        while(!file.eof())
        {
            file.getline(line, sizeof(line));
            if(strncmp(line, "VmRSS:", 6) == 0)
            {
                result = parseLine(line);
                break;
            }
        }
        file.close();
        // Unit (KB) To bytes
        result *= 1024;
    }
    else
        result = -1;
    return result;
}
#else // USE_PROCFS == 0

#include <sys/resource.h>

size_t get_memory_used()
{
    rusage rusg;

    // usage from kernel source
    if(getrusage(RUSAGE_SELF, &rusg))
    {
        return -1; // error
    }

    return rusg.ru_maxrss * 1024;
}
#endif

#endif

namespace RoninEngine::Perfomances
{
    // get an bytes
    RONIN_API size_t GetMemoryUsed()
    {
        size_t total;
#ifdef WIN32
        PROCESS_MEMORY_COUNTERS_EX pm;
        K32GetProcessMemoryInfo(GetCurrentProcess(), &pm, sizeof(pm));
        total = pm.WorkingSetSize;
#elif __unix__
        total = get_memory_used();
#endif
        return total;
    }

    RONIN_API int GetCPUCount()
    {
        // Get CPU Count from SDL API
        return SDL_GetCPUCount();
    }
} // namespace RoninEngine::Perfomances
