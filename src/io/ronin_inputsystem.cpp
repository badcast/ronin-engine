#include "ronin.h"

namespace RoninEngine::Runtime
{
    extern RoninInput internal_input;
    extern bool text_inputState;
    std::string internalText;

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

    const bool Input::GetMouseDown(MouseState state)
    {
        return internal_input._mouse_state[static_cast<int>(state)] & MouseStateFlags::MouseDown;
    }

    const bool Input::GetMouseUp(MouseState state)
    {
        return internal_input._mouse_state[static_cast<int>(state)] & MouseStateFlags::MouseUp;
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

    const bool Input::GetKeyDown(KeyboardCode code)
    {
        return static_cast<bool>(SDL_GetKeyboardState(nullptr)[code]);
    }

    const bool Input::GetKeyUp(KeyboardCode code)
    {
        return !static_cast<bool>(SDL_GetKeyboardState(nullptr)[code]);
    }

} // namespace RoninEngine::Runtime
