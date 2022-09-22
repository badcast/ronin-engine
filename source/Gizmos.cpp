#include "Gizmos.h"

#define USE_PRIMITIVES 1

#if USE_PRIMITIVES
#include <SDL2/SDL2_gfxPrimitives.h>
#endif

#include "framework.h"

namespace RoninEngine::Runtime {

float Gizmos::angle;

void internal_drawLine(Vec2 a, Vec2 b) {
    if (!Camera::mainCamera()) return;

    Vec2 p = Camera::mainCamera()->transform()->position();
    Vec2 dst;

    auto res = Application::getResolution();

    // dst.x = ((rect.w - dst.w) / 2.0f - (point->x + sourcePoint->x) *
    // squarePerPixels); dst.y = ((rect.h - dst.h) / 2.0f + (point->y -
    // sourcePoint->y) * squarePerPixels);

    // scalars

    Vec2 scale;
    SDL_RenderGetScale(Application::GetRenderer(), &scale.x, &scale.y);
    scale *= pixelsPerPoint;
    dst.x = res.width / 2.f;
    dst.y = res.height / 2.f;
    a.x = dst.x - (p.x - a.x) * scale.x;
    a.y = dst.y + (p.y - a.y) * scale.y;
    b.x = dst.x - (p.x - b.x) * scale.x;
    b.y = dst.y + (p.y - b.y) * scale.y;

    SDL_RenderDrawLineF(Application::GetRenderer(), a.x, a.y, b.x, b.y);
}

Color Gizmos::getColor() {
    Color clb;
    SDL_GetRenderDrawColor(Application::GetRenderer(), &clb.r, &clb.g, &clb.b, &clb.a);
    return clb;
}

void Gizmos::setColor(const Color& newColor) {
    SDL_SetRenderDrawColor(Application::GetRenderer(), newColor.r, newColor.g, newColor.b, newColor.a);
}

void Gizmos::DrawLine(Vec2 a, Vec2 b) { internal_drawLine(a, b); }
void drawBox() {
    Vec2 a, b;

    a.x = pixelsPerPoint;
    a.y = pixelsPerPoint;
    b.x = -pixelsPerPoint;
    b.y = pixelsPerPoint;
    internal_drawLine(std::move(a), std::move(b));

    a.x = pixelsPerPoint;
    a.y = -pixelsPerPoint;
    b.x = -pixelsPerPoint;
    b.y = -pixelsPerPoint;
    internal_drawLine(std::move(a), std::move(b));

    a.x = -pixelsPerPoint;
    a.y = pixelsPerPoint;
    b.x = -pixelsPerPoint;
    b.y = -pixelsPerPoint;
    internal_drawLine(std::move(a), std::move(b));

    a.x = pixelsPerPoint;
    a.y = pixelsPerPoint;
    b.x = pixelsPerPoint;
    b.y = -pixelsPerPoint;
    internal_drawLine(std::move(a), std::move(b));
}

void Gizmos::DrawPosition(const Vec2& origin, float scalar) {
    Vec2 a = Vec2::zero, b = Vec2::zero;

    // Draw Line H
    b = a = origin;
    a.x -= scalar;
    b.x += scalar;

    internal_drawLine(std::move(a), std::move(b));

    // Draw Line V
    b = a = origin;
    a.y -= scalar;
    b.y += scalar;
    internal_drawLine(std::move(a), std::move(b));
}

void Gizmos::DrawSquare(Vec2 origin, float width) { DrawRectangle(origin, width, width); }

void Gizmos::DrawRectangle(Vec2 origin, float width, float height) {
    origin = Camera::mainCamera()->WorldToScreenPoint(origin);
    std::uint16_t x, y;
    width *= pixelsPerPoint;
    height *= pixelsPerPoint;
    x = origin.x - width / 2;
    y = origin.y - height / 2;
#if USE_PRIMITIVES
    rectangleColor(Application::GetRenderer(), x, y, x + width, y + height, getColor());
#endif
}

void Gizmos::DrawSquareRounded(Vec2 origin, float width, std::uint16_t radius) {
    DrawRectangleRounded(origin, width, width, radius);
}

void Gizmos::DrawRectangleRounded(Vec2 origin, float width, float height, std::uint16_t radius) {
    origin = Camera::mainCamera()->WorldToScreenPoint(origin);
    std::uint16_t x, y;
    width *= pixelsPerPoint;
    height *= pixelsPerPoint;
    x = origin.x - width / 2;
    y = origin.y - height / 2;
#if USE_PRIMITIVES
    roundedRectangleColor(Application::GetRenderer(), x, y, x + width, y + height, radius, getColor());
#endif
}

void Gizmos::Draw2DWorldSpace(const Vec2& origin, int depth) {
    int i;
    Vec2 dest1, dest2;
    dest1 = dest2 = origin;
    Color lastColor = getColor();
    setColor(0xaa575757);

    // Draw H and V position
    DrawPosition(std::move(origin));

    for (i = 0; i < depth; ++i) {
        dest1 += Vec2::one;
        DrawPosition(std::move(dest1));

        dest2 += Vec2::minusOne;
        DrawPosition(std::move(dest2));
    }

    setColor(lastColor);
}

void Gizmos::DrawNavMesh(AIPathFinder::NavMesh* navMesh) {
    Vec2 lastPoint;
    Vec2 a, b;
    AIPathFinder::Neuron* p;
    Vec2Int p1, p2;
    Resolution res;
    Color prev;
    Color next;
    int yDefault;

    res = Application::getResolution();
    prev = getColor();
    setColor(next = 0xfff6f723);
    navMesh->GetNeuron(Camera::ScreenToWorldPoint(Vec2::zero), p1);
    navMesh->GetNeuron(Camera::ScreenToWorldPoint(Vec2(res.width, res.height)), p2);
    yDefault = p1.y;
    while (p1.x <= p2.x) {
        while (p1.y <= p2.y) {
            p = navMesh->GetNeuron(p1);
            lastPoint = navMesh->PointToWorldPosition(p1);
            if (!p || navMesh->neuronLocked(p1)) {
                next.r = 255;
                next.g = 0;
                next.b = 0;
            } else {
                next.r = 53;
                next.g = navMesh->neuronGetTotal(p1) ? 200 : 0;
                next.b = 246;
            }
            setColor(next);
            DrawPosition(lastPoint, 0.03f);
            ++p1.y;
        }
        p1.y = yDefault;
        ++p1.x;
    }
    setColor(prev);
}

void Gizmos::DrawTriangle(Vec2 origin, float base, float height) {
    Vec2 a, b;
    origin.y -= height / 2;
    a = b = origin;
    //  base /= 2.f;
    a.x -= base;
    b.x += base;
    origin.y += height;

    // draw base
    DrawLine(std::move(a), std::move(b));

    // draw left side
    DrawLine(std::move(a), std::move(origin));

    // draw right side
    DrawLine(std::move(b), std::move(origin));
}
void RoninEngine::Runtime::Gizmos::DrawTextOnPosition(Vec2 origin, const std::string& text) {
    origin = Camera::WorldToScreenPoint(origin);
    RoninEngine::UI::DrawFontAt(Application::GetRenderer(), text, 11, Vec2::RoundToInt(origin));
}
void RoninEngine::Runtime::Gizmos::DrawTextOnPosition_Legacy(Vec2 origin, const std::string& text) {
    Rect r;
    auto cam = Camera::mainCamera();
    // relative to
    origin = cam->WorldToScreenPoint(origin);
    r.x = static_cast<int>(origin.x);
    r.y = static_cast<int>(origin.y);

    UI::Free_Controls();
    UI::Render_String(Application::GetRenderer(), r, text.c_str(), text.length(), 2);
}

void Gizmos::DrawCircle(Vec2 origin, float distance) {
    origin = Camera::mainCamera()->WorldToScreenPoint(origin);
    std::uint16_t x, y, r;
    x = Math::number(origin.x);
    y = Math::number(origin.y);
    r = static_cast<std::uint16_t>(distance * pixelsPerPoint);
    Color m_color = getColor();
#if USE_PRIMITIVES
    circleRGBA(Application::GetRenderer(), x, y, r, m_color.r, m_color.g, m_color.b, m_color.a);
#endif
}

void Gizmos::DrawFill(Vec2 center, float width, float height) {
    center = Camera::WorldToScreenPoint(center);
    width *= pixelsPerPoint;
    height *= pixelsPerPoint;
    Rectf_t rect{center.x - width / 2, center.y - height / 2, width, height};

    SDL_RenderFillRectF(Application::GetRenderer(), reinterpret_cast<SDL_FRect*>(&rect));
}

void Gizmos::DrawFillSquare(Vec2 center, float width) {
    center = Camera::WorldToScreenPoint(center);
    width *= pixelsPerPoint;
    Rectf_t rect{center.x - width / 2, center.y - width / 2, width, width};

    SDL_RenderFillRectF(Application::GetRenderer(), reinterpret_cast<SDL_FRect*>(&rect));
}

void Gizmos::DrawStorm(Vec2 ray, int edges, int delim) {
    /*
    Описание данных stormMember
    Младшие 4 байта, это все для шаги

    stormMember low bits == steps
    stormMember high bits == maxSteps

    stormFlags = int 4 байта (32 бита)
    первые 3 байта (24 бита) = dimensions, от 0 до 0xFFFFFF значений
    остаток 1 байт (8 бит) stormFlags >> 24 = determinants (определители направлений луча)
    0xF000000    xDeterminant = stormFlags >> 24 & 0xF - горизонтальный детерминант x оси (абцис)
    0xF0000000   yDeterminant = stormFlags >> 28       - вертикальный детерминант y оси (ординат)
    */

    Vec2 last = ray;
    std::uint64_t stormMember = 0;
    std::int32_t stormFlags = 1;

    // draw current point

    Color lastColor = getColor();
    setColor(0xfff6f6f7);
    if (edges > 0)
        for (;;) {
            std::uint32_t&& steps = (stormMember & const_storm_steps_flag);
            std::uint32_t&& maxSteps = (stormMember >> 32);
            //Шаг заканчивается (step = turnSteps) происходит поворот
            if (steps % std::max(1u, (maxSteps / 4)) == 0) {
                //переход на новое измерение
                //при steps == maxsteps
                if (steps == maxSteps) {
                    if (--edges <= -1) break;

                    stormMember = (8ul * (stormFlags = stormFlags & const_storm_dimensions)) << 32;
                    stormFlags = ((stormFlags & const_storm_dimensions) + 1) | const_storm_yDeterminant_start;
                } else {
                    if (stormFlags >> 28) {
                        stormFlags ^= stormFlags & const_storm_xDeterminant;                                      // clear x
                        stormFlags |= ((stormFlags & const_storm_yDeterminant) >> 4) & const_storm_xDeterminant;  // x = y
                        stormFlags ^= stormFlags & const_storm_yDeterminant;                                      // clear y
                    } else {
                        stormFlags ^= stormFlags & const_storm_yDeterminant;                                      // clear y
                        stormFlags |= ((stormFlags & const_storm_xDeterminant) << 4) & const_storm_yDeterminant;  // y = x
                        stormFlags ^= const_storm_yDeterminant_inverse;                                           // inverse
                        stormFlags ^= stormFlags & const_storm_xDeterminant;                                      // clear x
                    }
                }
            }

            char&& xDeter = (stormFlags >> 24 & 0xf);
            char&& yDeter = stormFlags >> 28;
            ray.x += xDeter == 2 ? -1 : xDeter;
            ray.y += yDeter == 2 ? -1 : yDeter;

            DrawLine(last / delim, ray / delim);
            last = ray;

            if (!(stormMember & const_storm_steps_flag)) {
                if (yDeter) {
                    stormFlags ^= stormFlags & const_storm_xDeterminant;                                      // clear x
                    stormFlags |= ((stormFlags & const_storm_yDeterminant) >> 4) & const_storm_xDeterminant;  // x = y
                    stormFlags ^= stormFlags & const_storm_yDeterminant;                                      // clear y
                } else if (xDeter) {
                    stormFlags ^= stormFlags & const_storm_yDeterminant;                                      // clear y
                    stormFlags |= ((stormFlags & const_storm_xDeterminant) << 4) & const_storm_yDeterminant;  // y = x
                    stormFlags ^= stormFlags & const_storm_xDeterminant;                                      // clear x
                }
            }

            ++(*reinterpret_cast<std::uint32_t*>(&stormMember));
        }

    setColor(lastColor);
}

float Gizmos::square_triangle(float base, float height) { return base * height / 2; }
float Gizmos::square(float x) { return x * x; }
float Gizmos::square_rectangle(float a, float b) { return a * b; }
float Gizmos::square_circle(float radius) {
    /*
     *      get the square circle
     *      S = PI * r^2
     *      S = PI * (d/2)^2
     *      S = (PI * d^2) / 4
     *
     */
    return Math::PI * Math::pow2(radius);
}
float Gizmos::square_mesh(std::list<Vec2>&& vecs) {
    /*
     *    get the square a customize figure
     *    S = square
     *    S1 = cell square
     *    n = closed cell
     *    r = partially closed cell
     *    S = S1 * (n + 1/2 * r)
     *
     */

    float S = -1;

    // TODO: написать алгоритм измерение площади произвольным фигурам
    throw std::runtime_error("algorithm is not implemented");
    return S;
}
}  // namespace RoninEngine::Runtime
