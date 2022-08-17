#include "framework.h"

namespace RoninEngine {
Vec2Int input::m_mousePoint;
std::uint8_t input::mouseState = 0, input::lastMouseState, input::mouseWheels = 0;
Vec2 input::m_axis;

void input::Reset() {
    mouseState = 0;
    mouseWheels = 0;
}

void input::movement_update() {
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

void input::Update_Input(SDL_Event* e) {
    switch (e->type) {
        case SDL_EventType::SDL_MOUSEMOTION:
            m_mousePoint.x = e->motion.x;
            m_mousePoint.y = e->motion.y;
            break;
        case SDL_MOUSEWHEEL:
            mouseWheels = e->wheel.preciseY;
        break;
        case SDL_MOUSEBUTTONUP:
            // e->type == SDL_MOUSEBUTTONUP && button == event->button.button
            mouseState = e->button.button;
            break;
        case SDL_EventType::SDL_KEYDOWN: {
            if (false || e->key.repeat != 0) {
                switch (e->key.keysym.sym) {
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
        case SDL_EventType::SDL_KEYUP: {
            if (false && e->key.repeat == 0) {
                switch (e->key.keysym.sym) {
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

const bool input::isMouseUp(int button) { return mouseState == 1; }

const bool input::isMouseDown(int button) { return mouseState == 2; }

}  // namespace RoninEngine
