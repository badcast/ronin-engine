#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{
    enum MouseState { MouseLeft, MouseRight, MouseMiddle, MouseWheel };

    enum KeyboardState {
        // The CAPS LOCK light is on.
        CapsLock_On,
        // The key is enhanced.
        Enchanced_Key,
        // The left ALT key is pressed.
        LeftAlt_Pressed,
        // The left CTRL key is pressed.
        LeftCtrl_Pressed,
        // The NUM LOCK light is on.
        NumLock_On,
        // The right ALT key is pressed.
        RightAlt_Pressed,
        // The right CTRL key is pressed.
        RightCtrl_Pressed,
        // The SCROLL LOCK light is on.
        ScrollLock_On,
        // The SHIFT key is pressed.
        Shift_Pressed,
    };
    class RONIN_API Input
    {
    public:
        static const bool is_mouse_up(int button = 1);

        static const bool is_mouse_down(int button = 1);

        static const bool is_caps_lock();

        static const char wheel_radix();

        static const Vec2Int get_mouse_point();

        static const Vec2 get_mouse_pointf();

        static const Vec2 get_axis();

        static const bool get_key(SDL_Scancode code);

        static const bool get_key_down(SDL_Scancode code);
    };

} // namespace RoninEngine::Runtime
