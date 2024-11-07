#ifndef _RONIN_DEFINES_H_
#define _RONIN_DEFINES_H_ 1

// MSVC/MinGW Compiller
#if _MSC_VER || WIN32
#define API_EXPORT __declspec(dllexport)
#define API_IMPORT __declspec(dllimport)
#elif __linux__ || __unix__
// Linux/Unix GNU Compiller
#if defined(__GNUC__) && __GNUC__ >= 4
#define __GCCAPI__ __attribute__((visibility("default")))
#else
#define __GCCAPI__
#endif

#define API_EXPORT __GCCAPI__
#define API_IMPORT __GCCAPI__

#endif

#if RONIN_COMPILLING
#define RONIN_API API_EXPORT
#else
#define RONIN_API API_IMPORT
#endif

#endif
