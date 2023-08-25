
// MSVC/MinGW Compiller
#if _MSC_VER || WIN32
#define API_EXPORT __declspec(dllexport)
#define API_IMPORT __declspec(dllimport)
#elif __linux__ || __unix__
// Linux/Unix GNU Compiller
#if defined(__GNUC__) && __GNUC__ >= 4
#define __GLIBAPI__ __attribute__((visibility("default")))
#else
#define __GLIBAPI__
#endif

#define API_EXPORT __GLIBAPI__
#define API_IMPORT __GLIBAPI__

#endif

#if RONIN_COMPILLING
#define RONIN_API API_EXPORT
#else
#define RONIN_API API_IMPORT
#endif
