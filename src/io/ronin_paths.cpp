#include "ronin.h"

using namespace RoninEngine;

#ifdef WIN32
static const char seperator = '\\';
#elif __unix__ || __linux__
static const char seperator = '/';
#endif

std::string __organization__ {"Ronin Engine"};
std::string __application__ {"Framework"};

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
        RoninSimulator::ShowMessageFail(SDL_GetError());
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
        RoninSimulator::ShowMessageFail(SDL_GetError());
    }

    std::string str {prefPath};
    SDL_free(prefPath);

    return str;
}

char Paths::GetPathSeperatorOS()
{
    return seperator;
}
