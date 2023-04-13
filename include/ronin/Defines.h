
// MSVC Compiller
#if _MSC_VER || WIN32
#if RONIN_COMPILLING
#define SHARK __declspec(dllexport)
#else
#define SHARK __declspec(dllimport)
#endif
#elif __linux__ || __unix__
#define SHARK
#endif
