#pragma once

#include "Types.h"

namespace RoninEngine
{
    namespace UI
    {
        struct UIElement;
        class GUI;
        // using ui_callback = std::function<void(uid id, void *)>;
        // using ui_callback_integer = std::function<void(uid id, int)>;
        // using ui_callback_chars = std::function<void(uid id, const char *)>;
        // using ui_callback_float = std::function<void(uid id, float)>;
        // using ui_callback_void = std::function<void(uid id, void *)>;

        // main callback for elements
        typedef void (*ui_callback)(uid id, void *userdata);
        //  events
        typedef void (*ui_callback_integer)(uid id, int newValue);
        typedef void (*ui_callback_chars)(uid id, const char *newValue);
        typedef void (*ui_callback_float)(uid id, float newValue);
        typedef void (*ui_callback_void)(uid id);
    } // namespace UI
} // namespace RoninEngine
