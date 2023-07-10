#include "ronin.h"

namespace RoninEngine::Runtime
{
    extern Runtime::Vec2Int internal_mouse_point;
    extern std::uint8_t mouseState;
    extern std::uint8_t lastMouseState;
    extern std::uint8_t mouseWheels;
    extern Runtime::Vec2 m_axis;
    extern bool text_inputState;
    std::string internalText;

    void input_movement_update()
    {
        const uint8_t* s = SDL_GetKeyboardState(nullptr);
        if (s[SDL_SCANCODE_D] || s[SDL_SCANCODE_RIGHT])
            m_axis.x = 1;
        else if (s[SDL_SCANCODE_A] || s[SDL_SCANCODE_LEFT])
            m_axis.x = -1;
        else
            m_axis.x = 0;

        if (s[SDL_SCANCODE_W] || s[SDL_SCANCODE_UP])
            m_axis.y = 1;
        else if (s[SDL_SCANCODE_S] || s[SDL_SCANCODE_DOWN])
            m_axis.y = -1;
        else
            m_axis.y = 0;
    }

    void internal_update_input(SDL_Event* e)
    {
        switch (e->type)
        {
            case SDL_TEXTINPUT:
                internalText += e->text.text;
                break;
            case SDL_EventType::SDL_MOUSEMOTION:
                internal_mouse_point.x = e->motion.x;
                internal_mouse_point.y = e->motion.y;
                break;
            case SDL_MOUSEWHEEL:
                mouseWheels = e->wheel.y;
                break;
            case SDL_MOUSEBUTTONUP:
                mouseState = e->button.button;
                break;
            case SDL_EventType::SDL_KEYDOWN:
            {
                if (false || e->key.repeat != 0)
                {
                    switch (e->key.keysym.sym)
                    {
                        case SDL_KeyCode::SDLK_a:
                        case SDL_KeyCode::SDLK_LEFT:
                            m_axis.x = 1;
                            break;
                        case SDL_KeyCode::SDLK_d:
                        case SDL_KeyCode::SDLK_RIGHT:
                            m_axis.x = -1;
                            break;
                        case SDL_KeyCode::SDLK_w:
                        case SDL_KeyCode::SDLK_UP:
                            m_axis.y = 1;
                            break;
                        case SDL_KeyCode::SDLK_s:
                        case SDL_KeyCode::SDLK_DOWN:
                            m_axis.y = -1;
                            break;
                    }
                }
            }
            case SDL_EventType::SDL_KEYUP:
            {
                if (false && e->key.repeat == 0)
                {
                    switch (e->key.keysym.sym)
                    {
                        case SDL_KeyCode::SDLK_a:
                        case SDL_KeyCode::SDLK_LEFT:
                        case SDL_KeyCode::SDLK_d:
                        case SDL_KeyCode::SDLK_RIGHT:
                            m_axis.x = 0;
                            break;
                        case SDL_KeyCode::SDLK_w:
                        case SDL_KeyCode::SDLK_UP:
                        case SDL_KeyCode::SDLK_s:
                        case SDL_KeyCode::SDLK_DOWN:
                            m_axis.y = 0;
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

    void text_get(std::string& text)
    {
        text = internalText;
        internalText.resize(0);
    }

    const bool Input::is_mouse_up(int button)
    {
        return mouseState == button;
    }

    const bool Input::is_mouse_down(int button)
    {
        return SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LMASK != 0;
    }

    const char Input::wheel_radix()
    {
        return mouseWheels;
    }

    const Vec2Int Input::get_mouse_point()
    {
        return internal_mouse_point;
    }

    const Vec2 Input::get_mouse_pointf()
    {
        return { static_cast<float>(internal_mouse_point.x), static_cast<float>(internal_mouse_point.y) };
    }

    const Vec2 Input::get_axis()
    {
        return m_axis;
    }

    const bool Input::key_down(KeyboardCode code)
    {
        return static_cast<bool>(SDL_GetKeyboardState(nullptr)[code]);
    }

    const bool Input::key_up(KeyboardCode code)
    {
        return !static_cast<bool>(SDL_GetKeyboardState(nullptr)[code]);
    }

} // namespace RoninEngine
