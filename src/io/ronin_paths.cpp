#include "ronin.h"

#include "ronin_debug.h"

using namespace RoninEngine;

#if defined(_WIN32) || defined(WIN32)
static const char seperator = '\\';
#else
static const char seperator = '/';
#endif

// Set the default value for Org. App.

std::string __organization__ {"RoninEngine"};
std::string __application__ {"Owner"};

void Paths::RegOrg(const std::string &organization, const std::string &application)
{
    // TODO: Check valid string (member uses an Path/To/Application)
    __organization__ = organization;
    __application__ = application;
}

const std::string Paths::GetRuntimeDir()
{
    char *baseDir = SDL_GetBasePath();

    if(baseDir == nullptr)
    {
        ronin_err_sdl2();
        return "";
    }

    std::string str {baseDir};
    SDL_free(baseDir);

    return str;
}

const std::string Paths::GetPrefDir()
{
    char *prefPath = SDL_GetPrefPath(__organization__.c_str(), __application__.c_str());

    if(prefPath == nullptr)
    {
        ronin_err_sdl2();
        return "";
    }

    std::string str {prefPath};
    SDL_free(prefPath);

    return str;
}

char Paths::GetPathSeperatorOS()
{
    return seperator;
}

std::string Paths::MakePathValid(std::string path)
{
    for(size_t x = 0; x < path.length(); ++x)
    {
        char &c = path[x];
#if defined(_WIN32) || defined(WIN32)
        if(c == '/')
#else
        if(c == '\\')
#endif
        {
            c = seperator;
        }
    }

    return path;
}
