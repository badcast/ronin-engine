#include "ronin.h"

using namespace RoninEngine;

#ifdef WIN32
static const char seperator = '\\';
#elif __unix__ || __linux__
static const char seperator = '/';
#endif

static std::string __organization__ {"Ronin Engine"};
static std::string __application__ {"Framework"};

void Path::reg_application(const std::string &organization, const std::string &application)
{
    // TODO: Check valid string (member uses an Path/To/Application)
    __organization__ = organization;
    __application__ = application;
}

const std::string Path::app_dir()
{
    char *base_dir = SDL_GetBasePath();

    if(base_dir == nullptr)
    {
        RoninSimulator::ShowMessageFail(SDL_GetError());
    }

    std::string str {base_dir};
    SDL_free(base_dir);

    return str;
}

const std::string Path::pref_dir()
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

char Path::GetPathSeperatorOS()
{
    return seperator;
}
