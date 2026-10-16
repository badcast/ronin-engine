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
    if(!line)
        return 0;
    int i = strlen(line);
    if(i < 3)
        return 0;
    const char *p = line;
    while(*p && (*p < '0' || *p > '9'))
        p++;
    if(!*p)
        return 0;
    line[i - 3] = '\0';
    return atoi(p);
}

size_t get_memory_used()
{
    char line[128];
    size_t result = static_cast<size_t>(-1);
    std::ifstream file("/proc/self/status");
    if(file.is_open())
    {
        while(file.getline(line, sizeof(line)))
        {
            if(strncmp(line, "VmRSS:", 6) == 0)
            {
                result = static_cast<size_t>(parseLine(line)) * 1024;
                break;
            }
        }
        file.close();
    }
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

    RONIN_API int GetCPUUsage()
    {
#ifdef __linux__
        static unsigned long long prevTotal = 0, prevIdle = 0;
        std::ifstream file("/proc/stat");
        if(file.is_open())
        {
            std::string cpu;
            unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
            if(file >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal)
            {
                unsigned long long totalIdle = idle + iowait;
                unsigned long long total = user + nice + system + totalIdle + irq + softirq + steal;
                unsigned long long deltaTotal = total - prevTotal;
                unsigned long long deltaIdle = totalIdle - prevIdle;
                prevTotal = total;
                prevIdle = totalIdle;
                if(deltaTotal > 0)
                {
                    int usage = static_cast<int>(100 * (deltaTotal - deltaIdle) / deltaTotal);
                    return std::max(0, std::min(100, usage));
                }
            }
        }
#endif
        return 0;
    }
} // namespace RoninEngine::Perfomances
