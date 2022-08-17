#pragma once
#include "begin.h"

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
class RONIN_API input {
   private:
    static void Reset();

   public:
    static void Update_Input(SDL_Event*);

    static void movement_update();

    static const bool isMouseUp(int button = 1);

    static const bool isMouseDown(int button = 1);

    static const bool isCapsLock();

    static const char wheelRadix();

    static const Vec2Int getMousePoint();

    static const Vec2 getMousePointF();

    static const Vec2 get_axis();

    static const bool get_key(SDL_Scancode code);

    static const bool get_key_down(SDL_Scancode code);

    static void text_start_input();

    static void text_stop_input();

    static void text_get(std::string& text);
};

}  // namespace RoninEngine::Runtime
