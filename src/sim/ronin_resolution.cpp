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
        int result;
        int selDisplay;
        for(;;)
        {
            if(gscope.activeWindow == nullptr)
                result = SDL_GetNumVideoDisplays() > 0 ? 0 : -1;
            else
                result = SDL_GetWindowDisplayIndex(gscope.activeWindow);

            if((selDisplay = result) != -1)
            {
                result = SDL_GetNumDisplayModes(result);

                if(result > 0)
                {
                    int ndex = origin == 2 ? 0 : origin == 1 ? (result / 2 - 1) : result - 1;
                    result = SDL_GetDisplayMode(selDisplay, ndex, mode);
                }
            }
            break;
        }
        return result;
    }

    Vec2Int Resolution::GetSize() const
    {
        return Vec2Int {width, height};
    }

    Resolution Resolution::GetMaxResolution()
    {
        SDL_DisplayMode mode;
        if(enter_resolution(&mode, 2) < 0)
        {
            RoninSimulator::ShowMessageFail(SDL_GetError());
        }
        return Resolution {mode.w, mode.h, mode.refresh_rate};
    }

    Resolution Resolution::GetMinResolution()
    {
        SDL_DisplayMode mode;
        if(enter_resolution(&mode, 0) < 0)
        {
            RoninSimulator::ShowMessageFail(SDL_GetError());
        }
        return Resolution {mode.w, mode.h, mode.refresh_rate};
    }

    Resolution Resolution::GetMidResolution()
    {
        SDL_DisplayMode mode;
        if(enter_resolution(&mode, 1) < 0)
        {
            RoninSimulator::ShowMessageFail(SDL_GetError());
        }
        return Resolution {mode.w, mode.h, mode.refresh_rate};
    }
} // namespace RoninEngine
