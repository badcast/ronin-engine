#pragma once

#include "Types.h"

namespace RoninEngine
{
    namespace UI
    {
        struct UIElement;
        class GUI;
        // using UIEventUserData = std::function<void(uid id, void *)>;
        // using UIEventInteger = std::function<void(uid id, int)>;
        // using UIEventChar = std::function<void(uid id, const char *)>;
        // using UIEventFloat = std::function<void(uid id, float)>;
        // using UIEventVoid = std::function<void(uid id, void *)>;

        // main callback for elements
        typedef void (*UIEventUserData)(uid id, void *userdata);
        typedef void (*UIEventInteger)(uid id, int newValue);
        typedef void (*UIEventChar)(uid id, const char *newValue);
        typedef void (*UIEventFloat)(uid id, float newValue);
        typedef void (*UIEventBool)(uid id, bool newValue);
        typedef void (*UIEventVoid)(uid id);
    } // namespace UI
} // namespace RoninEngine
