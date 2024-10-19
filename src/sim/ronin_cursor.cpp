#include "ronin.h"
#include "ronin_debug.h"
#include "RoninCursor.h"

namespace RoninEngine::Runtime
{

    void RoninCursor::SetCursor(Cursor *cursor)
    {
        SDL_SetCursor(cursor);
    }

    bool RoninCursor::ShowCursor(bool toggle)
    {
        return SDL_ShowCursor(static_cast<int>(toggle)) == SDL_ENABLE;
    }

    bool RoninCursor::IsCursorShown()
    {
        return SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE;
    }

    bool RoninCursor::CaptureMouse(bool capture)
    {
        return SDL_CaptureMouse(static_cast<SDL_bool>(capture)) == 0;
    }

    Cursor *RoninCursor::GetCursor()
    {
        return SDL_GetCursor();
    }

    Cursor *RoninCursor::GetDefaultCursor()
    {
        return SDL_GetDefaultCursor();
    }

    Cursor *RoninCursor::GetSystemCursor(SystemCursorID id)
    {
        // Check range
        if(static_cast<int>(id) < 0 || static_cast<int>(id) >= SDL_NUM_SYSTEM_CURSORS)
            return nullptr;

        if(gscope.sysCursors.empty())
        {
            gscope.sysCursors.resize(static_cast<int>(SDL_NUM_SYSTEM_CURSORS), nullptr);
        }

        if(gscope.sysCursors[static_cast<int>(id)] == nullptr)
        {
            if((gscope.sysCursors[static_cast<int>(id)] = SDL_CreateSystemCursor(static_cast<SDL_SystemCursor>(static_cast<int>(id)))) == nullptr)
                Debug::Log(SDL_GetError());
        }

        return gscope.sysCursors[static_cast<int>(id)];
    }

} // namespace RoninEngine::Runtime
