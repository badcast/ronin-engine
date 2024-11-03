#pragma once

#include "Types.h"

namespace RoninEngine::Runtime
{
    enum DisplayType
    {
        Status,
        Warning,
        Error
    };

    class RONIN_API Debug final
    {
    public:
        static void Log(const std::string &message);
        static void Warn(const std::string &message);
        static void Error(const std::string &message);
        static void ShowDisplay(const std::string &title, const std::string &message, DisplayType type = DisplayType::Status);
    };
} // namespace RoninEngine::Runtime
