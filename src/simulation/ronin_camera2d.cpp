#define USE_OMP 0

#include "ronin.h"

#if USE_OMP
#include "omp.h"
#endif

// TODO: how to create shadow ? for all sprite renderer component
using namespace RoninEngine;

namespace RoninEngine::Runtime
{
    void native_render_2D(Camera2D* camera);

    inline bool area_cast(Renderer* target, const Vec2Int& wpLeftTop, const Vec2Int& wpRightBottom)
    {
        Vec2 rSz = target->get_size();
        Vec2 pos = target->transform()->position();
        return (pos.x + rSz.x >= wpLeftTop.x && pos.x - rSz.x <= wpRightBottom.x) && (pos.y - rSz.y <= wpLeftTop.y && pos.y + rSz.y >= wpRightBottom.y);
    }
    /*
        std::tuple<std::list<Renderer*>*, std::list<Light*>*> linear_select(Camera* camera)
        {
            constexpr std::uint8_t Nz = 2;
            list<Renderer*> layers[Nz];
            std::uint8_t zN = 0;
            if (__rendererOutResults.empty()) {
                auto res = Application::getResolution();
                Vec2 topLeft, rightBottom, rSz;
                topLeft = this->ScreenToWorldPoint(Vec2::zero);
                rightBottom = this->ScreenToWorldPoint(Vec2(res.width, res.height));

                for (auto render : RoninEngine::Level::getScene()->_assoc_renderers) {
                    if (render->zOrder >= Nz)
                        throw std::out_of_range("N z range");

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
                __lightsOutResults.assign(RoninEngine::Level::getScene()->_assoc_lightings.begin(), RoninEngine::Level::getScene()->_assoc_lightings.end());
            }

            return make_tuple(&__rendererOutResults, &__lightsOutResults);
        }
    */
    template <typename list_type_render = std::vector<Renderer*>, typename list_type_light = std::set<Light*>>
    inline std::tuple<std::map<int, list_type_render>*, list_type_light*> matrix_select(Camera2D* camera2d)
    {
        // TODO: Fixing renderer object and selec matrix element own Set T List
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

                Method finder: Storm                    see: Physics2D::storm_cast for details
                 ' * * * * * * * * * '
                 ' * * * * * * * * * '   n = 10
                 ' * * * * * * * * * '   n0 (first input point) = 0
                 ' * * * 2 3 4 * * * '   n10 (last input point) = 9
                 ' * * 9 1 0 5 * * * '
                 ' * * * 8 7 6 * * * '
                 ' * * * * * * * * * '
                 ' * * * * * * * * * '
                 ' * * * * * * * * * '
        */

        if (camera2d->camera_resources->renders.empty()) {
            Resolution res = active_resolution;
            Vec2Int wpLeftTop = Vec2::round_to_int(Camera::screen_to_world(Vec2::zero));
            Vec2Int wpRightBottom = Vec2::round_to_int(Camera::screen_to_world(Vec2(res.width, res.height)));
            Vec2 camera_position = camera2d->transform()->position();
            // RUN STORM CAST

            std::vector<Transform*> storm_result = Physics2D::sphere_cast<std::vector<Transform*>>(camera_position, Math::number(Math::max(wpRightBottom.x - camera_position.x, wpRightBottom.y - camera_position.y)) + 1 + camera2d->distanceEvcall);
            std::list<Renderer*> _removes;
            // собираем оставшиеся которые прикреплены к видимости
            for (auto x = std::begin(camera2d->camera_resources->prev); false && x != std::end(camera2d->camera_resources->prev); ++x) {
                if (area_cast(*x, wpLeftTop, wpRightBottom)) {
                    camera2d->camera_resources->renders[(*x)->transform()->layer].emplace_back((*x));
                } else {
                    //  _removes.emplace_back((*x));
                }
            }
            /**
                        for (Renderer* y : _removes)
                            camera2d->camera_resources->prev.erase(y);
            **/

            // order by layer component
            if constexpr (std::is_same<list_type_render, std::vector<Renderer*>>::value) {
                Renderer* r;
                const int size = storm_result.size();
                for (int x = 0; x < size; ++x) {
                    for (auto iter = ++std::begin(storm_result[x]->_owner->m_components), yiter = std::end(storm_result[x]->_owner->m_components); iter != yiter; ++iter) {
                        if (r = dynamic_cast<Renderer*>(*iter))
                            camera2d->camera_resources->renders[r->transform()->layer].push_back(r);
                    }
                }
            } else
                for (auto iter = std::begin(storm_result); iter != std::end(storm_result); ++iter) {
                    std::list<Renderer*> rends = (*iter)->game_object()->get_components<Renderer>();
                    for (Renderer* x : rends) {
                        camera2d->camera_resources->renders[x->transform()->layer].emplace_back(x);
                        // camera2d->camera_resources->prev.insert(x);
                    }
                }
        }

        if (camera2d->camera_resources->_lightsOutResults.empty()) {
            camera2d->camera_resources->_lightsOutResults.insert(World::self()->internal_resources->_assoc_lightings.begin(), World::self()->internal_resources->_assoc_lightings.end());
        }

        return std::make_tuple(&(camera2d->camera_resources->renders), &(camera2d->camera_resources->_lightsOutResults));
    }

    Camera2D::Camera2D()
        : Camera2D(DESCRIBE_AS_MAIN_OFF(Camera2D))
    {
    }

    Camera2D::Camera2D(const std::string& name)
        : Camera(DESCRIBE_AS_ONLY_NAME(Camera2D))
        , scale(Vec2::one)
        , visibleBorders(false)
        , visibleGrids(false)
        , visibleObjects(false)
    {
        DESCRIBE_AS_MAIN(Camera2D);
        distanceEvcall = 2;
    }

    Camera2D::Camera2D(const Camera2D& other)
        : Camera(other.m_name)
        , visibleBorders(other.visibleBorders)
        , visibleGrids(other.visibleGrids)
        , visibleObjects(other.visibleObjects)
        , scale(other.scale)
    {
    }

    void native_render_2D(Camera2D* camera)
    {

#if USE_OMP
        SDL_mutex* m = SDL_CreateMutex();
#endif
        Rendering wrapper;
        Transform* root_transform = World::self()->internal_resources->main_object->transform();
        Vec2 sourcePoint, camera_position = camera->transform()->_position;
        Color prevColor = Gizmos::get_color();

        Gizmos::set_color(0xffc4c4c4);
        if (camera->visibleGrids) {
            Gizmos::draw_2D_world_space(Vec2::zero);
            Gizmos::draw_position(camera_position, maxWorldScalar);
        }
        // get from matrix selection
        std::tuple<std::map<int, std::vector<Renderer*>>*, std::set<Light*>*> filter = matrix_select(camera);

        // scale.x = Mathf::Min(Mathf::Max(scale.x, 0.f), 1000.f);
        // scale.y = Mathf::Min(Mathf::Max(scale.y, 0.f), 1000.f);
        //_scale = scale*squarePerPixels;
        SDL_RenderSetScale(RoninEngine::renderer, camera->scale.x, camera->scale.y);

        // Render Objects

        std::map<int, std::vector<Renderer*>>* layer = std::get<0>(filter);
        for (auto iter = layer->begin(); iter != layer->end(); ++iter) {
            std::vector<Renderer*>& robject = iter->second;
            std::size_t size = robject.size();
#if USE_OMP
#pragma omp parallel for private(sourcePoint) private(wrapper)
#endif
            for (std::size_t pointer = 0; pointer < size; ++pointer) {
                Renderer* render_iobject = robject[pointer];
                Transform* render_transform = render_iobject->transform();
                memset(&wrapper, 0, sizeof(wrapper));

                render_iobject->render(&wrapper); // draw

                if (wrapper.texture) {

                    Transform* render_parent = render_transform->m_parent;
                    if (render_parent == root_transform) {
                        sourcePoint = render_transform->position();
                    } else {
                        // local position is direction
                        sourcePoint = Vec2::rotate_around(render_parent->_position, render_transform->local_position(), render_parent->angle() * Math::deg2rad);
                    }

                    wrapper.dst.w *= pixelsPerPoint; //_scale.x;
                    wrapper.dst.h *= pixelsPerPoint; //_scale.y;

                    Vec2 arranged = wrapper.dst.getXY();
                    if (arranged != Vec2::zero)
                        arranged = Vec2::rotate_around(sourcePoint, arranged, render_transform->angle() * Math::deg2rad);

                    sourcePoint += render_iobject->get_offset();

                    // convert world to screen

                    // Положение по горизонтале
                    wrapper.dst.x = arranged.x + ((active_resolution.width - wrapper.dst.w) / 2.0f - (camera_position.x - sourcePoint.x) * pixelsPerPoint);
                    // Положение по вертикале
                    wrapper.dst.y = arranged.y + ((active_resolution.height - wrapper.dst.h) / 2.0f + (camera_position.y - sourcePoint.y) * pixelsPerPoint);
#if USE_OMP
                    SDL_LockMutex(m);
#endif
                    // draw to backbuffer
                    SDL_RenderCopyExF(RoninEngine::renderer, wrapper.texture, (SDL_Rect*)&wrapper.src, reinterpret_cast<SDL_FRect*>(&wrapper.dst), render_transform->angle(), nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
#if USE_OMP
                    SDL_UnlockMutex(m);
#endif
                }
            }
        }

#if USE_OMP
        SDL_DestroyMutex(m);
#endif
#undef USE_OMP
        // TODO: Render light
        /*
                // Render Lights
                for (auto lightSource : *std::get<1>(filter)) {
                    // drawing
                    // clear
                    memset(&wrapper, 0, sizeof wrapper);

                    lightSource->get_light_source(&wrapper); // draw

                    if (wrapper.texture) {
                        Vec2 point = transform()->position();
                        sourcePoint = lightSource->transform()->position();

                        wrapper.dst.w *= pixelsPerPoint;

                        wrapper.dst.h *= pixelsPerPoint;

                        // h
                        wrapper.dst.x += ((rect.w - wrapper.dst.w) / 2.0f - (point.x - sourcePoint.x) * pixelsPerPoint);
                        // v
                        wrapper.dst.y += ((rect.h - wrapper.dst.h) / 2.0f + (point.y - sourcePoint.y) * pixelsPerPoint);

                        SDL_RenderCopyExF(renderer, wrapper.texture, reinterpret_cast<SDL_Rect*>(&wrapper.src), reinterpret_cast<SDL_FRect*>(&wrapper.dst), lightSource->transform()->_angle_ - transform()->_angle_, nullptr,
           SDL_RendererFlip::SDL_FLIP_NONE);
                    }
                }
        */
        if (camera->visibleBorders) {
            float offset = 25 * std::max(1 - TimeEngine::deltaTime(), 0.1f);
            float height = 200 * TimeEngine::deltaTime();

            wrapper.dst.x = ((active_resolution.width) / 2.0f);
            wrapper.dst.y = ((active_resolution.height) / 2.0f);

            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 25);

            // Center dot
            SDL_RenderDrawPointF(renderer, wrapper.dst.x, wrapper.dst.y);
            SDL_RenderDrawPointF(renderer, wrapper.dst.x - offset, wrapper.dst.y);
            SDL_RenderDrawPointF(renderer, wrapper.dst.x + offset, wrapper.dst.y);
            SDL_RenderDrawPointF(renderer, wrapper.dst.x, wrapper.dst.y - offset);
            SDL_RenderDrawPointF(renderer, wrapper.dst.x, wrapper.dst.y + offset);

            // borders
            Gizmos::draw_line(Vec2(offset, offset), Vec2(offset + height, offset));
            Gizmos::draw_line(Vec2(active_resolution.width - offset, offset), Vec2(active_resolution.width - offset - height, offset));
            Gizmos::draw_line(Vec2(offset, active_resolution.height - offset), Vec2(offset + height, active_resolution.height - offset));
            Gizmos::draw_line(Vec2(active_resolution.width - offset, active_resolution.height - offset), Vec2(active_resolution.width - offset - height, active_resolution.height - offset));

            Gizmos::draw_line(Vec2(offset, 1 + offset), Vec2(offset, offset + height));
            Gizmos::draw_line(Vec2(active_resolution.width - offset, 1 + offset), Vec2(active_resolution.width - offset, offset + height));
            Gizmos::draw_line(Vec2(offset, active_resolution.height - 1 - offset), Vec2(offset, active_resolution.height - offset - height));
            Gizmos::draw_line(Vec2(active_resolution.width - offset, active_resolution.height - 1 - offset), Vec2(active_resolution.width - offset, active_resolution.height - offset - height));
        }

        if (camera->visibleObjects || camera->visibleNames) {
            for (const auto& layer : (*std::get<0>(filter)))
                for (Renderer* face : layer.second) {

                    Vec2 p = face->transform()->position() + face->get_offset();
                    if (camera->visibleObjects) {
                        Rect factSz = face->get_relative_size();
                        Vec2 sz = Vec2::scale(face->get_size(), Vec2(factSz.getWH()) / pixelsPerPoint);
                        Gizmos::set_color(Color::blue);
                        Gizmos::draw_rectangle_rounded(p, sz.x, sz.y, 10);
                        Gizmos::set_color(Color::black);
                        Gizmos::draw_position(p, 0.2f);
                    }
                    if (camera->visibleNames) {
                        Gizmos::set_color(Color::white);
                        Gizmos::draw_text(p, face->game_object()->m_name);
                    }
                }
        }

        Gizmos::set_color(prevColor);
    }

} // namespace RoninEngine::Runtime
