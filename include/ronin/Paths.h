#pragma once

#include "begin.h"

namespace RoninEngine
{
    class RONIN_API Path
    {
    public:
        // Set Orgazniation and Application member use.
        static void reg_application(const std::string& organization, const std::string& application);
        // Get Execute Application directory
        static const std::string app_dir();
        // Get preference directory
        static const std::string pref_dir();
    };
}
