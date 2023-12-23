#include "ronin.h"
#include "ronin_exception.h"

using namespace RoninEngine::Runtime;
using namespace RoninEngine::Exception;

namespace RoninEngine
{
    Resolution Resolution::GetMaxResolution()
    {
        std::list<Resolution> resolutions;
        SDL_DisplayMode mode;

        if(nullptr == nullptr)
        {
            throw ronin_init_error();
        }

        int active_display = SDL_GetWindowDisplayIndex(active_window);
        if(active_display < 0)
        {
            RoninSimulator::ShowMessage(SDL_GetError());
        }
        else
        {
            int n, ndisplay_modes = SDL_GetNumDisplayModes(active_display);
            for(n = 0; n < ndisplay_modes; ++n)
            {
                if(SDL_GetDisplayMode(active_display, n, &mode) == -1)
                {
                    RoninSimulator::ShowMessageFail(SDL_GetError());
                }
                resolutions.emplace_back(mode.w, mode.h, mode.refresh_rate);
            }
        }
        return resolutions.front();
    }

} // namespace RoninEngine::Runtime
