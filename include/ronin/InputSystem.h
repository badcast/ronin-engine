#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{
    /* Mouse State */
    enum MouseState
    {
        MouseLeft = 0,
        MouseMiddle = 1,
        MouseRight = 2,
        MouseX1 = 3,
        MouseX2 = 4
    };

    enum class KeyboardState
    {
        // The CAPS LOCK light is on.
        CapsLock = 57,
        // The NUM LOCK light is on.
        NumLock = 83,
        // The SCROLL LOCK light is on.
        ScrollLock = 71,
    };

    // This Code applyies from SDL_SCANCODE
    enum KeyboardCode
    {
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

    /**
     * @brief The Input class provides methods for handling user input within the RONIN game engine.
     */
    class RONIN_API Input
    {
    public:
        /**
         * @brief Check if a mouse button is currently in a pressed state.
         * @param state The state of the mouse button to check.
         * @return True if the mouse button is currently pressed, false otherwise.
         */
        static const bool GetMouseDown(MouseState state);

        /**
         * @brief Check if a mouse button is currently in a released state.
         * @param state The state of the mouse button to check.
         * @return True if the mouse button is currently released, false otherwise.
         */
        static const bool GetMouseUp(MouseState state);

        /**
         * @brief Get the current mouse wheel scroll amount.
         * @return The amount of mouse wheel scroll.
         */
        static const int GetMouseWheel();

        /**
         * @brief Get the current mouse pointer coordinates as integers.
         * @return A Vec2Int containing the x and y coordinates of the mouse pointer.
         */
        static const Vec2Int GetMousePoint();

        /**
         * @brief Get the current mouse pointer coordinates as floating-point values.
         * @return A Vec2 containing the x and y coordinates of the mouse pointer.
         */
        static const Vec2 GetMousePointf();

        /**
         * @brief Get the current state of a specific axis.
         * @return A Vec2 representing the state of the specified axis.
         */
        static const Vec2 GetAxis();

        /**
         * @brief Check the state of a keyboard key.
         * @param state The state of the keyboard key to check.
         * @return True if the keyboard key is currently in the specified state, false otherwise.
         */
        static const bool GetKeyState(KeyboardState state);

        /**
         * @brief Check if a keyboard key is currently in a pressed state.
         * @param code The code of the keyboard key to check.
         * @return True if the keyboard key is currently pressed, false otherwise.
         */
        static const bool GetKeyDown(KeyboardCode code);

        /**
         * @brief Check if a keyboard key is currently in a released state.
         * @param code The code of the keyboard key to check.
         * @return True if the keyboard key is currently released, false otherwise.
         */
        static const bool GetKeyUp(KeyboardCode code);
    };

} // namespace RoninEngine::Runtime
