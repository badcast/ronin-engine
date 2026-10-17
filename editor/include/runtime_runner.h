#pragma once

#include <string>
#include "editor_state.h"

namespace RoninEditor
{
    class RuntimeRunner
    {
    public:
        // Runs the given EditorProject inside the native RoninSimulator (SDL2 window).
        // If maxFrames > 0, automatically requests quit after maxFrames frames (useful for tests).
        static int runProject(const EditorProject &project, int initialWorldIndex = -1, int maxFrames = 0);

        // Loads a .roninproj file from disk and runs it in RoninSimulator.
        static int runProjectFromFile(const std::string &projectFilePath, int initialWorldIndex = -1, int maxFrames = 0);
    };
} // namespace RoninEditor
