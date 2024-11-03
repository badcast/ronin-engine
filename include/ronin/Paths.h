#pragma once

#include "begin.h"

namespace RoninEngine
{
    class RONIN_API Paths
    {
    public:
        // Set Orgazniation and Application member use.
        static void RegOrg(const std::string &organization, const std::string &application);
        // Get Runtime Application directory
        static const std::string GetRuntimeDir();
        // Get preference directory
        static const std::string GetPrefDir();
        // Result on Linux "/"
        // Result on Windows "\"
        static char GetPathSeperatorOS();
        // Make valid paths
        static std::string MakePathValid(std::string path);
    };
} // namespace RoninEngine
