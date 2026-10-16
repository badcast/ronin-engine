#include "ronin.h"
#include "ronin_exception.h"

using namespace RoninEngine::Runtime;
using namespace RoninEngine::Exception;

namespace RoninEngine
{
    // origin = 0 - min
    // origin = 1 - middle
    // origin = 2 - max
    int enter_resolution(SDL_DisplayMode *mode, int origin = 0)
    {
        int result = -1;
        int selDisplay;
        if(gscope.activeWindow == nullptr)
            selDisplay = SDL_GetNumVideoDisplays() > 0 ? 0 : -1;
        else
            selDisplay = SDL_GetWindowDisplayIndex(gscope.activeWindow);

        if(selDisplay != -1)
        {
            int numModes = SDL_GetNumDisplayModes(selDisplay);
            if(numModes > 0)
            {
                int ndex = origin == 2 ? 0 : origin == 1 ? std::max(0, numModes / 2) : numModes - 1;
                ndex = std::max(0, std::min(numModes - 1, ndex));
                result = SDL_GetDisplayMode(selDisplay, ndex, mode);
            }
        }
        return result;
    }

    Vec2Int Resolution::GetSize() const
    {
        return Vec2Int {width, height};
    }

    Resolution Resolution::GetMaxResolution()
    {
        SDL_DisplayMode mode {0, 1920, 1080, 60, nullptr};
        if(enter_resolution(&mode, 2) < 0)
        {
            RoninSimulator::ShowMessageFail(SDL_GetError());
        }
        return Resolution {mode.w, mode.h, mode.refresh_rate};
    }

    Resolution Resolution::GetMinResolution()
    {
        SDL_DisplayMode mode {0, 640, 480, 60, nullptr};
        if(enter_resolution(&mode, 0) < 0)
        {
            RoninSimulator::ShowMessageFail(SDL_GetError());
        }
        return Resolution {mode.w, mode.h, mode.refresh_rate};
    }

    Resolution Resolution::GetMidResolution()
    {
        SDL_DisplayMode mode {0, 1280, 720, 60, nullptr};
        if(enter_resolution(&mode, 1) < 0)
        {
            RoninSimulator::ShowMessageFail(SDL_GetError());
        }
        return Resolution {mode.w, mode.h, mode.refresh_rate};
    }
} // namespace RoninEngine
