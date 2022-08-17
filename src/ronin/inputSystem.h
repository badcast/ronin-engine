#pragma once

#include "dependency.h"

namespace RoninEngine::Runtime {
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
class input {
   private:
    friend class RoninEngine::Application;
    static Vec2Int m_mousePoint;
    static std::uint8_t mouseState;
    static std::uint8_t lastMouseState;
    static std::uint8_t mouseWheels;
    static Vec2 m_axis;

    static void Reset();

   public:
    static void Init_Input() {}

    static void Update_Input(SDL_Event*);

    static void movement_update();

    static const bool isMouseUp(int button = 1);
    static const bool isMouseDown(int button = 1);
    static const bool isCapsLock() { return get_key(SDL_SCANCODE_CAPSLOCK); }

    static const char wheelRadix() { return mouseWheels; }

    static const Vec2Int getMousePoint() { return m_mousePoint; }

    static const Vec2 getMousePointF() { return {static_cast<float>(m_mousePoint.x), static_cast<float>(m_mousePoint.y)}; }

    static const Vec2 get_axis() { return m_axis; }

    static const bool get_key(SDL_Scancode code) { return static_cast<bool>(SDL_GetKeyboardState(nullptr)[code]); }
    static const bool get_key_down(SDL_Scancode code) { return static_cast<bool>(SDL_GetKeyboardState(nullptr)[code]); }
};

}  // namespace RoninEngine
