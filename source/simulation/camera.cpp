#include "ronin.h"

static RoninEngine::Runtime::Camera* _main = nullptr;

namespace RoninEngine::Runtime {

Camera::Camera() : Camera(typeid(*this).name()) {}
Camera::Camera(const std::string& name) : Component(name) {
    //using this camera as main
    _main = this;
    targetClear = enabled = true;
    distanceEvcall = 2;
}
Camera::~Camera() {
    //release using pointer
    if (_main == this) _main = nullptr;
}

bool Camera::isFocused() { return _main == this; }

void Camera::Focus() { _main = this; }

/*
std::tuple<list<Renderer*>*, list<Light*>*> linearSelection() {
    constexpr std::uint8_t Nz = 2;
    list<Renderer*> layers[Nz];
    std::uint8_t zN = 0;
    if (__rendererOutResults.empty()) {
        auto res = Application::getResolution();
        Vec2 topLeft, rightBottom, rSz;
        topLeft = this->ScreenToWorldPoint(Vec2::zero);
        rightBottom = this->ScreenToWorldPoint(Vec2(res.width, res.height));

        for (auto render : RoninEngine::Level::getScene()->_assoc_renderers) {
            if (render->zOrder >= Nz) throw std::out_of_range("N z range");

            rSz = render->GetSize() / 2;

            Transform* t = render->transform();
            if (render->zOrder >= 0 &&
                //	X
                (t->_p.x + rSz.x >= topLeft.x && t->_p.x - rSz.x <= rightBottom.x) &&
                //	Y
                (t->_p.y - rSz.y <= topLeft.y && t->_p.y + rSz.y >= rightBottom.y)) {
                layers[render->zOrder].emplace_front(render);
            }
        }

            // ordering and collect
            list<Renderer*>* target;
        while ((target = zN < Nz ? &layers[zN++] : nullptr)) {
            for (auto x = begin(*target); x != end(*target); ++x) {
                __rendererOutResults.emplace_back((*x));
            }
        }
    }

    if (__lightsOutResults.empty()) {
        __lightsOutResults.assign(RoninEngine::Level::getScene()->_assoc_lightings.begin(),
                                  RoninEngine::Level::getScene()->_assoc_lightings.end());
    }

    return make_tuple(&__rendererOutResults, &__lightsOutResults);
}
*/

std::set<Renderer*> prev;
inline bool areaCast(Renderer* target, const Vec2Int& wpLeftTop, const Vec2Int& wpRightBottom) {
    Vec2 rSz = target->getSize();
    Vec2 pos = target->transform()->position();
    return (pos.x + rSz.x >= wpLeftTop.x && pos.x - rSz.x <= wpRightBottom.x) &&
           (pos.y - rSz.y <= wpLeftTop.y && pos.y + rSz.y >= wpRightBottom.y);
}
std::tuple<std::map<int, std::set<Renderer*>>*, std::set<Light*>*> Camera::matrixSelection() {
    /*       This is projection: Algorithm storm member used.
            x-------------------
            |                   |      = * - is Vector2 (point)
            |  r * * * * * * *  |      = r - current point (ray)
            |  * * * * * * * *  |      = x - wpLeftTop
            |  * * * * * * * *  |      = y - wpRightBottom
            |  * * * * * * * *  |
            |  * * * * * * * *  |
            |  * * * * * * * *  |
            |  * * * * * * * *  |
            |                   |
             -------------------y

            Method finder: Storm                                    see: Physics2D::stormCast for details
             ' * * * * * * * * *'
             ' * * * * * * * * *'   n = 10
             ' * * * * * * * * *'   n0 (first input point) = 0
             ' * * * 2 3 4 * * *'   n10 (last input point) = 9
             ' * * 9 1 0 5 * * *'
             ' * * * 8 7 6 * * *'
             ' * * * * * * * * *'
             ' * * * * * * * * *'
             ' * * * * * * * * *'
    */

    if (renders.empty()) {
        Resolution res = Application::getResolution();
        Vec2Int wpLeftTop = Vec2::RoundToInt(ScreenToWorldPoint(Vec2::zero));
        Vec2Int wpRightBottom = Vec2::RoundToInt(ScreenToWorldPoint(Vec2(res.width, res.height)));
        //RUN STORM CAST
        std::list<Transform*> result = Physics2D::stormCast(
            transform()->p, Math::number(Math::max(wpRightBottom.x - transform()->p.x, wpRightBottom.y - transform()->p.y)) +
                                1 + distanceEvcall);

        std::list<Renderer*> _removes;
        //собираем оставшиеся которые прикреплены к видимости
        for (auto x = std::begin(prev); x != std::end(prev); ++x) {
            if (areaCast(*x, wpLeftTop, wpRightBottom)) {
                renders[(*x)->transform()->layer].insert((*x));
            } else {
                _removes.emplace_back((*x));
            }
        }

        for (Renderer* y : _removes) prev.erase(y);

        // order by layer component

        for (auto iter = std::begin(result); iter != std::end(result); ++iter) {
            std::list<Renderer*> rends = (*iter)->gameObject()->getComponents<Renderer>();
            if (!rends.empty()) {
                for (auto x : rends) {
                    renders[x->transform()->layer].insert(x);
                }

                prev.insert(rends.begin(), rends.end());
            }
        }
    }

    if (__lightsOutResults.empty()) {
        __lightsOutResults.insert(RoninEngine::Level::self()->_assoc_lightings.begin(),
                                  RoninEngine::Level::self()->_assoc_lightings.end());
    }

    return make_tuple(&renders, &__lightsOutResults);
}

const Vec2 Camera::ScreenToWorldPoint(Vec2 screenPoint) {
    Resolution res = Application::getResolution();
    Vec2 offset = _main->transform()->position();
    Vec2 scale;
    SDL_RenderGetScale(Application::GetRenderer(), &scale.x, &scale.y);
    scale *= pixelsPerPoint;
    screenPoint.x = ((res.width / 2.f - screenPoint.x) * -1) / scale.x;
    screenPoint.y = (res.height / 2.f - screenPoint.y) / scale.y;
    screenPoint += offset;
    return screenPoint;
}
const Vec2 Camera::WorldToScreenPoint(Vec2 worldPoint) {
    Resolution res = Application::getResolution();
    Vec2 scale;
    Vec2 offset = _main->transform()->position();
    SDL_RenderGetScale(Application::GetRenderer(), &scale.x, &scale.y);
    scale *= pixelsPerPoint;
    //Horizontal position
    worldPoint.x = (res.width / 2.f - (offset.x - worldPoint.x) * scale.x);
    //Vertical position
    worldPoint.y = (res.height / 2.f + (offset.y - worldPoint.y) * scale.y);
    return worldPoint;
}

const Vec2 Camera::ViewportToWorldPoint(Vec2 viewportPoint) {
    Resolution res = Application::getResolution();
    Vec2 scale, offset = _main->transform()->position();
    SDL_RenderGetScale(Application::GetRenderer(), &scale.x, &scale.y);
    scale *= pixelsPerPoint;
    // Horizontal position
    viewportPoint.x = (res.width / 2.f - res.width * viewportPoint.x) * -1 / scale.x;
    // Vertical position
    viewportPoint.y = (res.height / 2.f - res.height * viewportPoint.y) / scale.y;
    viewportPoint += offset;
    return viewportPoint;
}

const Vec2 Camera::WorldToViewport(Vec2 worldPoint) {
    Resolution res = Application::getResolution();
    Vec2 scale;
    Vec2 offset = _main->transform()->position();
    SDL_RenderGetScale(Application::GetRenderer(), &scale.x, &scale.y);
    scale *= pixelsPerPoint;
    // Horizontal position
    worldPoint.x = (res.width / 2.0f - (offset.x - worldPoint.x) * scale.x) / res.width;
    // Vertical position
    worldPoint.y = (res.height / 2.0f + (offset.y - worldPoint.y) * scale.y) / res.height;
    return worldPoint;
}

const Vec2 Camera::WorldToViewportClamp(Vec2 worldPoint) {
    worldPoint = WorldToViewport(worldPoint);
    worldPoint.x = Math::Clamp01(worldPoint.x);
    worldPoint.y = Math::Clamp01(worldPoint.y);
    return worldPoint;
}
Camera* Camera::mainCamera() { return _main; }

}  // namespace RoninEngine::Runtime
