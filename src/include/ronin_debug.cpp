
#include "ronin_debug.h"

using namespace RoninEngine::Runtime;

void Debug::Log(const std::string &message)
{
    ronin_log(message.c_str());
}

void Debug::Warn(const std::string &message)
{
    ronin_warn(message.c_str());
}

void Debug::Error(const std::string &message)
{
    ronin_err(message.c_str());
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

void ronin_log(const char *msg)
{
    SDL_Log("log: %s", msg);
}

void ronin_warn(const char *msg)
{
    SDL_LogWarn(SDL_LOG_PRIORITY_WARN, msg);
}

void ronin_err(const char *msg)
{
    SDL_LogError(SDL_LOG_PRIORITY_ERROR,msg);
}
