#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{
    /* Mouse Button */
    enum MouseButton
    {
        MouseLeft = 0,
        MouseMiddle = 1,
        MouseRight = 2,
        MouseX1 = 3,
        MouseX2 = 4
    };

    // This Code applyies from SDL_SCANCODE
    enum KeyboardCode
    {
        KB_UNKNOWN = 0,
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
        KB_NONUSHASH = 50,
        KB_SEMICOLON = 51,
        KB_APOSTROPHE = 52,
        KB_GRAVE = 53,
        KB_COMMA = 54,
        KB_PERIOD = 55,
        KB_SLASH = 56,

        KB_CAPSLOCK = 57,

        KB_F1 = 58,
        KB_F2 = 59,
        KB_F3 = 60,
        KB_F4 = 61,
        KB_F5 = 62,
        KB_F6 = 63,
        KB_F7 = 64,
        KB_F8 = 65,
        KB_F9 = 66,
        KB_F10 = 67,
        KB_F11 = 68,
        KB_F12 = 69,

        KB_PRINTSCREEN = 70,
        KB_SCROLLLOCK = 71,
        KB_PAUSE = 72,
        KB_INSERT = 73,
        KB_HOME = 74,
        KB_PAGEUP = 75,
        KB_DELETE = 76,
        KB_END = 77,
        KB_PAGEDOWN = 78,
        KB_RIGHT = 79,
        KB_LEFT = 80,
        KB_DOWN = 81,
        KB_UP = 82,

        KB_NUMLOCKCLEAR = 83,
        KB_KP_DIVIDE = 84,
        KB_KP_MULTIPLY = 85,
        KB_KP_MINUS = 86,
        KB_KP_PLUS = 87,
        KB_KP_ENTER = 88,
        KB_KP_1 = 89,
        KB_KP_2 = 90,
        KB_KP_3 = 91,
        KB_KP_4 = 92,
        KB_KP_5 = 93,
        KB_KP_6 = 94,
        KB_KP_7 = 95,
        KB_KP_8 = 96,
        KB_KP_9 = 97,
        KB_KP_0 = 98,
        KB_KP_PERIOD = 99,

        KB_LCTRL = 224,
        KB_LSHIFT = 225,
        KB_LALT = 226,
        KB_LGUI = 227,
        KB_LWIN = KB_LGUI,
        KB_RCTRL = 228,
        KB_RSHIFT = 229,
        KB_RALT = 230,
        KB_RGUI = 231,
        KB_RWIN = KB_RGUI
    };

    struct JoystickInfo
    {
        std::string name;
        std::string guid;
        std::string devicePath;
    };

    /**
     * @brief The Input class provides methods for handling user input within the RoninEngine.
     */
    class RONIN_API Input
    {
    public:
        /**
         * @brief Check if a mouse button is currently in a pressed state.
         * @param state The state of the mouse button to check.
         * @return True if the mouse button is currently pressed, false otherwise.
         */
        static const bool GetMouseDown(MouseButton state);

        /**
         * @brief Check if a mouse button is currently in a released state.
         * @param state The state of the mouse button to check.
         * @return True if the mouse button is currently released, false otherwise.
         */
        static const bool GetMouseUp(MouseButton state);

        /**
         * @brief Get the current mouse wheel scroll amount.
         * @return The amount of mouse wheel scroll.
         */
        static const int GetMouseWheel();

        /**
         * @brief Get the current mouse pointer coordinates as integers.
         * @return A Vec2Int containing the x and y coordinates of the mouse pointer from Screen.
         */
        static const Vec2Int GetMousePoint();

        /**
         * @brief Get the current mouse pointer coordinates as floating-point values.
         * @return A Vec2 containing the x and y coordinates of the mouse pointer from Screen.
         */
        static const Vec2 GetMousePointf();

        /**
         * @brief Get the current state of a specific axis.
         * @return A Vec2 representing the state of the specified axis.
         */
        static const Vec2 GetAxis();

        /**
         * @brief Check if a keyboard key is currently in a pressed state.
         * @param keyCode The keyCode of the keyboard key to check.
         * @return True if the keyboard key is currently pressed, false otherwise.
         */
        static const bool GetKeyDown(KeyboardCode keyCode);

        /**
         * @brief Check if a keyboard key is currently in a released state.
         * @param keyCode The code of the keyboard key to check.
         * @return True if the keyboard key is currently released, false otherwise.
         */
        static const bool GetKeyUp(KeyboardCode keyCode);

        /**
         * @brief Get name of the Keyboard Code
         * @param keyCode The code value for name.
         * @see KeyboardCode
         * @return A name of the Key
         */
        static std::string GetKeyName(int keyCode);

        static std::vector<JoystickInfo> GetJoysticks();
    };

} // namespace RoninEngine::Runtime
