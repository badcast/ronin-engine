#include "ronin.h"

namespace RoninEngine::Runtime
{
    extern RoninInput internal_input;
    extern bool text_inputState;
    std::string internalText;

    constexpr struct
    {
        int dead_zones[2][2] {{1, 3}, {100, 223}};
        char key_strings[105][16] {
            // KB_UNKNOWN = 0,
            "None",
            // KB_A = 4,
            "A",
            // KB_B = 5,
            "B",
            // KB_C = 6,
            "C",
            // KB_D = 7,
            "D",
            // KB_E = 8,
            "E",
            // KB_F = 9,
            "F",
            // KB_G = 10,
            "G",
            // KB_H = 11,
            "H",
            // KB_I = 12,
            "I",
            // KB_J = 13,
            "J",
            // KB_K = 14,
            "K",
            // KB_L = 15,
            "L",
            // KB_M = 16,
            "M",
            // KB_N = 17,
            "N",
            // KB_O = 18,
            "O",
            // KB_P = 19,
            "P",
            // KB_Q = 20,
            "Q",
            // KB_R = 21,
            "R",
            // KB_S = 22,
            "S",
            // KB_T = 23,
            "T",
            // KB_U = 24,
            "U",
            // KB_V = 25,
            "V",
            // KB_W = 26,
            "W",
            // KB_X = 27,
            "X",
            // KB_Y = 28,
            "Y",
            // KB_Z = 29,
            "Z",

            // KB_1 = 30,
            "1",
            // KB_2 = 31,
            "2",
            // KB_3 = 32,
            "3",
            // KB_4 = 33,
            "4",
            // KB_5 = 34,
            "5",
            // KB_6 = 35,
            "6",
            // KB_7 = 36,
            "7",
            // KB_8 = 37,
            "8",
            // KB_9 = 38,
            "9",
            // KB_0 = 39,
            "0",

            // KB_RETURN = 40,
            "Return",
            // KB_ESCAPE = 41,
            "ESC",
            // KB_BACKSPACE = 42,
            "Backspace",
            // KB_TAB = 43,
            "TAB",
            // KB_SPACE = 44,
            "Space",

            // KB_MINUS = 45,
            "MINUS",
            // KB_EQUALS = 46,
            "EQUALS",
            // KB_LEFTBRACKET = 47,
            "LBRACKET",
            // KB_RIGHTBRACKET = 48,
            "RBRACKET",
            // KB_BACKSLASH = 49,
            "BACKSLASH",
            // KB_NONUSHASH = 50,
            "NONUSHASH",
            // KB_SEMICOLON = 51,
            "SEMICOLON",
            // KB_APOSTROPHE = 52,
            "APOSTROPHE",
            // KB_GRAVE = 53,
            "GRAVE",
            // KB_COMMA = 54,
            "COMMA",
            // KB_PERIOD = 55,
            "PERIOD",
            // KB_SLASH = 56,
            "SLASH",

            // KB_CAPSLOCK = 57,
            "CapsLock",

            // KB_F1 = 58,
            "F1",
            // KB_F2 = 59,
            "F2",
            // KB_F3 = 60,
            "F3",
            // KB_F4 = 61,
            "F4",
            // KB_F5 = 62,
            "F5",
            // KB_F6 = 63,
            "F6",
            // KB_F7 = 64,
            "F7",
            // KB_F8 = 65,
            "F8",
            // KB_F9 = 66,
            "F9",
            // KB_F10 = 67,
            "F10",
            // KB_F11 = 68,
            "F11",
            // KB_F12 = 69,
            "F12",

            // KB_PRINTSCREEN = 70,
            "PrintScreen",
            // KB_SCROLLLOCK = 71,
            "ScrollLock",
            // KB_PAUSE = 72,
            "Pause",
            // KB_INSERT = 73,
            "Insert",
            // KB_HOME = 74,
            "Home",
            // KB_PAGEUP = 75,
            "PageUp",
            // KB_DELETE = 76,
            "Delete",
            // KB_END = 77,
            "End",
            // KB_PAGEDOWN = 78,
            "PageDown",
            // KB_RIGHT = 79,
            "Right",
            // KB_LEFT = 80,
            "Left",
            // KB_DOWN = 81,
            "Down",
            // KB_UP = 82,
            "Up",

            // KB_NUMLOCKCLEAR = 83,
            "Numlock",
            // KB_KP_DIVIDE = 84,
            "Divide",
            // KB_KP_MULTIPLY = 85,
            "Multiply",
            // KB_KP_MINUS = 86,
            "Minus",
            // KB_KP_PLUS = 87,
            "Plus",
            // KB_KP_ENTER = 88,
            "Enter",
            // KB_KP_1 = 89,
            "1",
            // KB_KP_2 = 90,
            "2",
            // KB_KP_3 = 91,
            "3",
            // KB_KP_4 = 92,
            "4",
            // KB_KP_5 = 93,
            "5",
            // KB_KP_6 = 94,
            "6",
            // KB_KP_7 = 95,
            "7",
            // KB_KP_8 = 96,
            "8",
            // KB_KP_9 = 97,
            "9",
            // KB_KP_0 = 98,
            "0",
            // KB_KP_PERIOD = 99,
            "Period",

            // KB_LCTRL = 224,
            "LCtrl",
            // KB_LSHIFT = 225,
            "LShift",
            // KB_LALT = 226,
            "LAlt",
            // KB_LGUI = 227,
            "LGUI",
            // KB_RCTRL = 228,
            "RCtrl",
            // KB_RSHIFT = 229,
            "RShift",
            // KB_RALT = 230,
            "RAlt",
            // KB_RGUI = 231,
            "RGUI",
        };

        const int clast_keyCode = KeyboardCode::KB_RGUI;
    } c_key_codename;

    void input_movement_update()
    {
        const uint8_t *s = SDL_GetKeyboardState(nullptr);
        if(s[SDL_SCANCODE_D] || s[SDL_SCANCODE_RIGHT])
            internal_input._movement_axis.x = 1;
        else if(s[SDL_SCANCODE_A] || s[SDL_SCANCODE_LEFT])
            internal_input._movement_axis.x = -1;
        else
            internal_input._movement_axis.x = 0;

        if(s[SDL_SCANCODE_W] || s[SDL_SCANCODE_UP])
            internal_input._movement_axis.y = 1;
        else if(s[SDL_SCANCODE_S] || s[SDL_SCANCODE_DOWN])
            internal_input._movement_axis.y = -1;
        else
            internal_input._movement_axis.y = 0;
    }

    void internal_update_input(SDL_Event *e)
    {
        switch(e->type)
        {
            case SDL_TEXTINPUT:
                internalText += e->text.text;
                break;
            case SDL_EventType::SDL_MOUSEMOTION:
                internal_input._mouse_position.x = e->motion.x;
                internal_input._mouse_position.y = e->motion.y;
                break;
            case SDL_MOUSEWHEEL:
                internal_input._mouse_wheels = e->wheel.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                internal_input._mouse_state[e->button.button - 1] = MouseStateFlags::MouseDown;
                break;
            case SDL_MOUSEBUTTONUP:
                internal_input._mouse_state[e->button.button - 1] = MouseStateFlags::MouseUp;
                break;
            case SDL_EventType::SDL_KEYDOWN:
            {
                if(e->key.repeat != 0)
                {
                    switch(e->key.keysym.sym)
                    {
                        case SDL_KeyCode::SDLK_a:
                        case SDL_KeyCode::SDLK_LEFT:
                            internal_input._movement_axis.x = 1;
                            break;
                        case SDL_KeyCode::SDLK_d:
                        case SDL_KeyCode::SDLK_RIGHT:
                            internal_input._movement_axis.x = -1;
                            break;
                        case SDL_KeyCode::SDLK_w:
                        case SDL_KeyCode::SDLK_UP:
                            internal_input._movement_axis.y = 1;
                            break;
                        case SDL_KeyCode::SDLK_s:
                        case SDL_KeyCode::SDLK_DOWN:
                            internal_input._movement_axis.y = -1;
                            break;
                    }
                }
            }
            case SDL_EventType::SDL_KEYUP:
            {
                if(false && e->key.repeat == 0)
                {
                    switch(e->key.keysym.sym)
                    {
                        case SDL_KeyCode::SDLK_a:
                        case SDL_KeyCode::SDLK_LEFT:
                        case SDL_KeyCode::SDLK_d:
                        case SDL_KeyCode::SDLK_RIGHT:
                            internal_input._movement_axis.x = 0;
                            break;
                        case SDL_KeyCode::SDLK_w:
                        case SDL_KeyCode::SDLK_UP:
                        case SDL_KeyCode::SDLK_s:
                        case SDL_KeyCode::SDLK_DOWN:
                            internal_input._movement_axis.y = 0;
                            break;
                    }
                }
            }
        }
    }

    void text_start_input()
    {
        //    SDL_IsTextInputActive for check state
        SDL_StartTextInput();
    }

    void text_stop_input()
    {
        SDL_StopTextInput();
    }

    void text_get(std::string &text)
    {
        text = internalText;
        internalText.resize(0);
    }

    const bool Input::GetMouseDown(MouseButton state)
    {
        return internal_input._mouse_state[static_cast<int>(state)] & MouseStateFlags::MouseDown != 0;
    }

    const bool Input::GetMouseUp(MouseButton state)
    {
        return internal_input._mouse_state[static_cast<int>(state)] & MouseStateFlags::MouseUp != 0;
    }

    const int Input::GetMouseWheel()
    {
        return internal_input._mouse_wheels;
    }

    const Vec2Int Input::GetMousePoint()
    {
        return internal_input._mouse_position;
    }

    const Vec2 Input::GetMousePointf()
    {
        return Vec2 {internal_input._mouse_position};
    }

    const Vec2 Input::GetAxis()
    {
        return internal_input._movement_axis;
    }

    const bool Input::GetKeyState(KeyboardState state)
    {
        return static_cast<bool>(SDL_GetKeyboardState(nullptr)[static_cast<int>(state)]);
    }

    const bool Input::GetKeyDown(KeyboardCode keyCode)
    {
        return static_cast<bool>(SDL_GetKeyboardState(nullptr)[keyCode]);
    }

    const bool Input::GetKeyUp(KeyboardCode keyCode)
    {
        return !static_cast<bool>(SDL_GetKeyboardState(nullptr)[keyCode]);
    }

    const char *Input::GetKeyName(int keyCode)
    {
        constexpr int c_key_len = (sizeof(c_key_codename.key_strings) / sizeof(c_key_codename.key_strings[0]));

        // It's unknown state
        if(keyCode <= 0 || keyCode > c_key_codename.clast_keyCode)
        {
            keyCode = 0;
        }
        else
        {
            int x, c_dz_len = (sizeof(c_key_codename.dead_zones) / sizeof(c_key_codename.dead_zones[0]));
            int key = keyCode;

            // foreach dead zones
            for(x = 0; x < c_dz_len; ++x)
            {
                if(key >= c_key_codename.dead_zones[x][0])
                {
                    // if the enter zone
                    if(key <= c_key_codename.dead_zones[x][1])
                    {
                        // zoned: result of the unknown
                        keyCode = 0;
                        break;
                    }
                    if(key >= c_key_codename.dead_zones[x][1])
                        keyCode -= c_key_codename.dead_zones[x][1]; // push next zone up
                }
                else
                {
                    break; // is out range
                }
            }
        }

        return c_key_codename.key_strings[keyCode];
    }

} // namespace RoninEngine::Runtime
