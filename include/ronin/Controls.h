#pragma once

#include "Types.h"

namespace RoninEngine
{
    namespace UI
    {
        struct UIElement;
        class GUI;

        // main callback for elements
        typedef void (*ui_callback)(uid id, void *userdata);
        // events
        typedef void (*ui_callback_integer)(uid id, int newValue);
        typedef void (*ui_callback_chars)(uid id, const char *newValue);
        typedef void (*ui_callback_float)(uid id, float newValue);
        typedef void (*ui_callback_void)(uid id);
    } // namespace UI
} // namespace RoninEngine
