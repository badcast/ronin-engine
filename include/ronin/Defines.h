
// MSVC Compiller
#if _MSC_VER || WIN32
#define API_EXPORT __declspec(dllexport)
#define API_IMPORT __declspec(dllimport)
#if RONIN_COMPILLING
#define RONIN_API API_EXPORT
#else
#define RONIN_API API_IMPORT
#endif
#elif __linux__ || __unix__
#define API_EXPORT
#define API_IMPORT
#define RONIN_API
#endif
