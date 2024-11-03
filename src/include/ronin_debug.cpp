#include "ronin_debug.h"

using namespace RoninEngine::Runtime;

LogLevel rloglevel = LogLevel::Backend;

void ronin_log(const char *fmt, ...)
{
    SDL_Log("log: %s", fmt);
}

void ronin_warn(const char *fmt, ...)
{
    std::va_list args;
    // std::va_start(args, )
    SDL_LogWarn(SDL_LOG_PRIORITY_WARN, fmt);
}

void ronin_err(const char *fmt, ...)
{
    SDL_LogError(SDL_LOG_PRIORITY_ERROR, fmt);
}

void ronin_err_d(const char *fmt, ...)
{
    SDL_LogError(SDL_LOG_PRIORITY_ERROR, fmt);

    Debug::ShowDisplay("Exception", fmt, DisplayType::Error);
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
