#include "ronin.h"

#ifdef __linux__
#include <cstdio>
#include <sys/sysinfo.h>
#include <sys/types.h>
#endif

#if WIN32
typedef struct _PROCESS_MEMORY_COUNTERS_EX {
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
typedef PROCESS_MEMORY_COUNTERS_EX* PPROCESS_MEMORY_COUNTERS_EX;

extern "C" {
__declspec(dllimport) void* __stdcall GetCurrentProcess(void);
__declspec(dllimport) int __stdcall K32GetProcessMemoryInfo(void* Process, PROCESS_MEMORY_COUNTERS_EX* ppsmemCounters, unsigned long cb);
}
#elif __unix__

int parseLine(char* line)
{
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*(p) < '0' || *(p) > '9')
        p++;
    line[i - 3] = '\0';
    i = atoi(p);
    return i;
}

int get_memory_used()
{
    char line[128];
    int result = -1;
    FILE* file = fopen("/proc/self/status", "r");

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result * 1024;
}

#endif

/*

TODO: Общая доступная виртуальная память
TODO: Используемая в настоящее время виртуальная память
TODO: Виртуальная память, используемая в данный момент моим процессом

TODO: Всего доступно RAM
TODO: RAM в настоящее время используется
TODO: RAM в настоящее время используется этим процессом

TODO: % CPU используется в настоящее время
TODO: % CPU в настоящее время используется этим процессом

*/

const size_t get_process_sizeMemory()
{
#ifdef WIN32
    PROCESS_MEMORY_COUNTERS_EX pm;
    K32GetProcessMemoryInfo(GetCurrentProcess(), &pm, sizeof(pm));
    size_t total = pm.WorkingSetSize;
#elif __unix__

    size_t total = get_memory_used();
#endif
    return total;
}
