#pragma once

#include "Types.h"

namespace RoninEngine
{
    namespace UI
    {
        struct UIElement;
        class GUI;

        // main callback for elements
        typedef void (*ui_callback)(uid id, void* userdata);

        // events
        typedef void (*event_index_changed)(uid id, int selectedIndex);

        typedef void (*event_value_changed)(uid id, float value);
    } // namespace UI
} // namespace RoninEngine
