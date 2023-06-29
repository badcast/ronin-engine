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

    // This Code applyies from SDL_SCANCODE
    enum KeyboardCode {
        KB_A = 4,
        KB_B = 5,
        KB_C = 6,
        KB_D = 7,
        KB_E = 8,
        KB_F = 9,
        KB_G = 10,
        KB_H = 11,
        KB_I = 12,
        KB_J = 13,
        KB_K = 14,
        KB_L = 15,
        KB_M = 16,
        KB_N = 17,
        KB_O = 18,
        KB_P = 19,
        KB_Q = 20,
        KB_R = 21,
        KB_S = 22,
        KB_T = 23,
        KB_U = 24,
        KB_V = 25,
        KB_W = 26,
        KB_X = 27,
        KB_Y = 28,
        KB_Z = 29,

        KB_1 = 30,
        KB_2 = 31,
        KB_3 = 32,
        KB_4 = 33,
        KB_5 = 34,
        KB_6 = 35,
        KB_7 = 36,
        KB_8 = 37,
        KB_9 = 38,
        KB_0 = 39,

        KB_RETURN = 40,
        KB_ESCAPE = 41,
        KB_BACKSPACE = 42,
        KB_TAB = 43,
        KB_SPACE = 44,

        KB_MINUS = 45,
        KB_EQUALS = 46,
        KB_LEFTBRACKET = 47,
        KB_RIGHTBRACKET = 48,
        KB_BACKSLASH = 49,

        KB_LCTRL = 224,
        KB_LSHIFT = 225,
        KB_LALT = 226,
        KB_LGUI = 227,
        KB_RCTRL = 228,
        KB_RSHIFT = 229,
        KB_RALT = 230,
        KB_RGUI = 231,
    };

    class RONIN_API Input
    {
    public:
        static const bool is_mouse_up(int button = 1);

        static const bool is_mouse_down(int button = 1);

        static const char wheel_radix();

        static const Vec2Int get_mouse_point();

        static const Vec2 get_mouse_pointf();

        static const Vec2 get_axis();

        static const bool key_down(KeyboardCode code);

        static const bool key_up(KeyboardCode code);
    };

} // namespace RoninEngine::Runtime
