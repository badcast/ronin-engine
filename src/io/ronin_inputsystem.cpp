#include "ronin.h"
#include "ronin_debug.h"

namespace RoninEngine::Runtime
{
    extern RoninInput internal_input;
    extern bool text_inputState;
    std::string internalText;

    void internal_input_init()
    {
        int n;
        const std::uint8_t *keys = SDL_GetKeyboardState(&n);

        if(keys == nullptr)
        {
            ronin_err_sdl2();
            return;
        }

        internal_input.prev_frame_keys = static_cast<std::uint8_t *>(RoninMemory::mem_alloc(n));
        memcpy((internal_input.prev_frame_keys) + sizeof(std::uint16_t), keys, n);
    }

    void internal_input_update(const SDL_Event &e)
    {
        switch(e.type)
        {
            case SDL_TEXTINPUT:
                internalText += e.text.text;
                break;
            case SDL_MOUSEMOTION:
                internal_input._mouse_position.x = e.motion.x;
                internal_input._mouse_position.y = e.motion.y;
                break;
            case SDL_MOUSEWHEEL:
                internal_input._mouse_wheels = e.wheel.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                internal_input._mouse_state[e.button.button - 1] = InputStateFlags::KeyDown;
                break;
            case SDL_MOUSEBUTTONUP:
                internal_input._mouse_state[e.button.button - 1] = InputStateFlags::KeyUp;
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                internal_input.prev_frame_keys[e.key.keysym.scancode] = e.type - SDL_KEYDOWN + 1;
                break;
            }
        }
    }

    void internal_input_release()
    {
        if(internal_input.prev_frame_keys != nullptr)
        {
            RoninMemory::mem_free(internal_input.prev_frame_keys);
            internal_input.prev_frame_keys = nullptr;
        }
    }

    void internal_input_update_before()
    {
        constexpr int JoystickDeadZone = 8000;

        // Update Game Axis on events

        const Uint8 *keys = SDL_GetKeyboardState(nullptr);

        SDL_Joystick *joystick = nullptr;

        if(SDL_NumJoysticks() > 0)
        {
            joystick = SDL_JoystickOpen(0);
        }

        if(keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
            internal_input._movement_axis.x = -1;
        else if(keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
            internal_input._movement_axis.x = 1;
        else if(joystick)
        {
            Sint16 v = SDL_JoystickGetAxis(joystick, 0);
            internal_input._movement_axis.x = v > JoystickDeadZone ? 1 : v < -JoystickDeadZone ? -1 : 0;
        }
        else
            internal_input._movement_axis.x = 0;

        if(keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
            internal_input._movement_axis.y = 1;
        else if(keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
            internal_input._movement_axis.y = -1;
        else if(joystick)
        {
            Sint16 v = SDL_JoystickGetAxis(joystick, 1);
            internal_input._movement_axis.y = v > JoystickDeadZone ? -1 : v < -JoystickDeadZone ? 1 : 0;
        }
        else
            internal_input._movement_axis.y = 0;

        if(joystick)
            SDL_JoystickClose(joystick);
    }

    void internal_input_update_after()
    {

        // TODO: Optimize here (List Key Downs)

        std::replace_if(internal_input._mouse_state, internal_input._mouse_state + sizeof(internal_input._mouse_state), [](auto val) { return val == InputStateFlags::KeyUp; }, 0);
        std::replace_if(internal_input.prev_frame_keys, internal_input.prev_frame_keys + SDL_NUM_SCANCODES, [](auto val) { return val == InputStateFlags::KeyUp; }, 0);
        internal_input._mouse_wheels = 0;

        /* int n; const std::uint8_t *keys = SDL_GetKeyboardState(&n);  if(keys == nullptr || n == 0) return;
         * if(internal_input.prev_frame_keys == nullptr || (*reinterpret_cast<std::uint16_t *>(internal_input.prev_frame_keys)) != n) {
         * internal_input.prev_frame_keys = static_cast<std::uint8_t *>(RoninMemory::ronin_memory_realloc(internal_input.prev_frame_keys, n
         * + sizeof(std::uint16_t))); *reinterpret_cast<std::uint16_t *>(internal_input.prev_frame_keys) = static_cast<std::uint16_t>(n); }
         * memcpy((internal_input.prev_frame_keys) + sizeof(std::uint16_t), keys, n); */
    }

    void text_get(std::string &text)
    {
        text = internalText;
        internalText.resize(0);
    }

    const bool Input::GetMouseDown(MouseButton state)
    {
        return internal_input._mouse_state[static_cast<int>(state)] == InputStateFlags::KeyDown;
    }

    const bool Input::GetMouseUp(MouseButton state)
    {
        return internal_input._mouse_state[static_cast<int>(state)] == InputStateFlags::KeyUp;
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

    const bool Input::GetKeyDown(KeyboardCode keyCode)
    {
        return internal_input.prev_frame_keys[keyCode] == InputStateFlags::KeyDown;
    }

    const bool Input::GetKeyUp(KeyboardCode keyCode)
    {
        return internal_input.prev_frame_keys[keyCode] == InputStateFlags::KeyUp;
    }

#define VLO(X) (X & 0xFFFF)
#define VHI(X) (X >> 16)
#define VCR(X, Y) ((X & 0xFFFF) | (Y << 16))

    constexpr struct
    {
        int dead_zones[2] {VCR(1, 3), VCR(100, 223)};
        char key_strings[105][16] {
            // KB_UNKNOWN = 0,
            "None",
            /*
                { Black Hole  1 ... 3 }
            */
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
            /*
                { Black Hole  100 ... 223 }
            */
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

    constexpr int c_key_len = (sizeof(c_key_codename.key_strings) / sizeof(c_key_codename.key_strings[0]));
    constexpr int c_dz_len = (sizeof(c_key_codename.dead_zones) / sizeof(c_key_codename.dead_zones[0]));

    std::string Input::GetKeyName(int keyCode)
    {
        if(keyCode <= 0 || keyCode > c_key_codename.clast_keyCode)
        {
            keyCode = 0;
        }
        else
        {
            int i, x, y, key;
            for(i = 0, key = keyCode; i < c_dz_len; ++i)
            {
                x = VLO(c_key_codename.dead_zones[i]);
                if(key < x)
                {
                    // Key find
                    break;
                }

                y = VHI(c_key_codename.dead_zones[i]);
                if(key <= y)
                {
                    // zoned: result of the unknown
                    keyCode = 0;
                    break;
                }
                keyCode -= y - x + 1;
            }
        }

        return std::string{c_key_codename.key_strings[keyCode]};
    }

    std::vector<JoystickInfo> Input::GetJoysticks()
    {
        int nums;
        SDL_Joystick *jsk;
        SDL_JoystickGUID jsk_guid;
        std::vector<JoystickInfo> joysticks;
        JoystickInfo append;

        nums = SDL_NumJoysticks();

        if(nums < 0)
            ronin_err_sdl2();

        while(nums-- > 0)
        {
            jsk = SDL_JoystickOpen(nums);

            if(!jsk)
            {
                ronin_err_sdl2();
                continue;
            }

            // Entry info
            append.name = SDL_JoystickName(jsk);
            append.devicePath = SDL_JoystickPath(jsk);

            jsk_guid = SDL_JoystickGetGUID(jsk);
            append.guid.resize(33);
            SDL_JoystickGetGUIDString(jsk_guid, append.guid.data(), append.guid.size());

            SDL_JoystickClose(jsk);

            joysticks.emplace_back(append);
        }

        return joysticks;
    }

} // namespace RoninEngine::Runtime
