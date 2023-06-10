#include "ronin.h"

#ifdef __linux__
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

typedef struct {
    std::size_t virtualMem;
    std::size_t physicalMem;
    std::size_t totalRam;
    std::size_t totalSwap;
    std::size_t cpuCount;
    char cpuName[32];
} system_info;

std::size_t unix_proc_parse(char* line)
{
    // This assumes that a digit will be found and the line ends in " Kb".
    std::size_t i;
    const char* p = line;
    while (*p < '0' || *p > '9')
        p++;
    //  line[i - 3] = '\0';
    i = atoi(p);
    return i;
}

system_info unix_process_info_from_proc()
{
    system_info upm;
    const char* fself = "/proc/self/status";
    FILE* f = fopen(fself, "r");

    char buffer[64];
    if (f == nullptr)
        RoninEngine::Application::fail("unix: invalid read \"" + std::string(fself) + "\" access denied");
    while (fgets(buffer, static_cast<int>(sizeof(buffer)), f) != nullptr) {
        if (!strncmp(buffer, "VmSize:", 7)) {
            upm.virtualMem = unix_proc_parse(buffer) * 1024; // read in KB
            continue;
        }
        if (!strncmp(buffer, "VmRSS:", 6)) {
            upm.physicalMem = unix_proc_parse(buffer) * 1024; // read in KB
            break;
        }
    }

    fclose(f);

    return upm;
}
system_info unix_process_info()
{
    system_info upm;
    struct sysinfo s;

    // get
    ::sysinfo(&s);

    upm.totalRam = s.totalram;
    upm.totalSwap = s.totalswap;

    upm.physicalMem = s.totalswap - s.freeswap;
    upm.physicalMem *= s.mem_unit;

    upm.virtualMem = s.totalram - s.freeram;
    upm.virtualMem *= s.mem_unit;
    return upm;
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

const size_t get_process_privateMemory()
{

#ifdef WIN32
    PROCESS_MEMORY_COUNTERS_EX pm;
    K32GetProcessMemoryInfo(GetCurrentProcess(), &pm, sizeof(pm));
    size_t total = pm.PrivateUsage;
#elif __unix__
    size_t total = unix_process_info_from_proc().virtualMem;
#endif
    return total;
}

const size_t get_process_sizeMemory()
{
#ifdef WIN32
    PROCESS_MEMORY_COUNTERS_EX pm;
    K32GetProcessMemoryInfo(GetCurrentProcess(), &pm, sizeof(pm));
    size_t total = pm.WorkingSetSize;
#elif __unix__
    size_t total = unix_process_info_from_proc().physicalMem;
#endif
    return total;
}