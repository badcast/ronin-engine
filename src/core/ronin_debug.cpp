#include <cstdarg>

#include "ronin_debug.h"

using namespace RoninEngine::Runtime;

LogLevel rloglevel = LogLevel::Backend;

void ronin_log(const char *msg)
{
    SDL_Log("%s", msg);
}

void ronin_warn(const char *msg)
{
    SDL_LogWarn(SDL_LOG_PRIORITY_WARN, "%s", msg);
}

void ronin_err(const char *msg)
{
    SDL_LogError(SDL_LOG_PRIORITY_ERROR, "%s", msg);
}

void ronin_err_d(const char *msg)
{
    Debug::ShowDisplay("Exception", msg, DisplayType::Error);
}

void Debug::Log(const std::string &message)
{
    rloglevel = LogLevel::Frontend;
    ronin_log(message.c_str());
    rloglevel = LogLevel::Backend;
}

void Debug::Warn(const std::string &message)
{
    rloglevel = LogLevel::Frontend;
    ronin_warn(message.c_str());
    rloglevel = LogLevel::Backend;
}

void Debug::Error(const std::string &message)
{
    rloglevel = LogLevel::Frontend;
    ronin_err(message.c_str());
    rloglevel = LogLevel::Backend;
}

void Debug::ShowDisplay(const std::string &title, const std::string &message, DisplayType type)
{
    int displayFlagWnd;
    if(type == DisplayType::Warning)
        displayFlagWnd = SDL_MessageBoxFlags::SDL_MESSAGEBOX_WARNING;
    else if(type == DisplayType::Error)
        displayFlagWnd = SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR;
    else
        displayFlagWnd = SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION;

    SDL_ShowSimpleMessageBox(displayFlagWnd, title.c_str(), message.c_str(), gscope.activeWindow);
}

void ronin_err_sdl2()
{
    ronin_err(SDL_GetError());
}
