#include "ronin.h"
#include "ronin_matrix.h"
#include "ronin_render_cache.h"

using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    struct
    {
        Rendering wrapper;
        Transform *root_transform;
        Vec2 sourcePoint, camera_position;
        Vec2Int wpLeftTop;
        Vec2Int wpRightBottom;
        std::map<int, std::vector<Renderer *>> orders;
        int edges;
    } params;

    void native_render_2D(Camera2D *camera);
    // void native_render_3D(Camera3D *camera);

    inline bool area_cast(Renderer *target, const Vec2Int &wpLeftTop, const Vec2Int &wpRightBottom)
    {
        Vec2 rSz = target->getSize();
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
                __lightsOutResults.assign(RoninEngine::Level::getScene()->_assoc_lightings.begin(),
       RoninEngine::Level::getScene()->_assoc_lightings.end());
            }

            return make_tuple(&__rendererOutResults, &__lightsOutResults);
        }
    */

    Camera2D::Camera2D() : Camera2D(DESCRIBE_AS_MAIN_OFF(Camera2D))
    {
    }

    Camera2D::Camera2D(const std::string &name) : Camera(DESCRIBE_AS_ONLY_NAME(Camera2D)), visibleBorders(false), visibleGrids(false), visibleObjects(false)
    {
        DESCRIBE_AS_MAIN(Camera2D);
        distanceEvcall = 1;
    }

    Camera2D::Camera2D(const Camera2D &other) : Camera(other.m_name), visibleBorders(other.visibleBorders), visibleGrids(other.visibleGrids), visibleObjects(other.visibleObjects)
    {
    }

    void Camera2D::SetZoom(float value)
    {
        value = Math::Clamp<float>(value, 0, 100);
        res->scale = Vec2::one * value;
    }

    float Camera2D::GetZoom()
    {
        return res->scale.magnitude();
    }

    void native_render_2D(Camera2D *camera)
    {
        camera->res->culled = 0;

        params.camera_position = camera->transform()->position();
        params.root_transform = World::GetCurrentWorld()->irs->mainObject->transform();
        params.wpLeftTop = Vec2::RoundToInt(Camera::ScreenToWorldPoint(Vec2::zero));
        params.wpRightBottom = Vec2::RoundToInt(Camera::ScreenToWorldPoint(Vec2(gscope.activeResolution.width, gscope.activeResolution.height)));
        params.edges = Math::Number(Math::Max(params.wpRightBottom.x - params.camera_position.x, params.wpRightBottom.y - params.camera_position.y)) + 5 + camera->distanceEvcall;

        // Clearing
        if(camera->backclear)
        {
            RenderUtility::SetColor(camera->backcolor);
            SDL_RenderClear(gscope.renderer);
        }

        if(camera->visibleGrids)
        {
            RenderUtility::Draw2DWorldSpace(Vec2::zero);
            RenderUtility::DrawPosition(params.camera_position, defaultMaxWorldScalar);
        }

        // scale.x = Mathf::Min(Mathf::Max(scale.x, 0.f), 1000.f);
        // scale.y = Mathf::Min(Mathf::Max(scale.y, 0.f), 1000.f);
        //_scale = scale*squarePerPixels;

        // camera->res->offsetScaling = Vec2::Scale(Vec2::up_right,Vec2::one * defaultPixelsPerPoint);

        SDL_RenderSetScale(gscope.renderer, camera->res->scale.x, camera->res->scale.y);

        // get from matrix selection
        /* RUN STORM CAST
           + - - - - - - - - - +    \
           ' → → → → → → → → ↓ '        \
           ' ↑ → → → → → → ↓ ↓ '            \
           ' ↑ ↑ → → → → ↓ ↓ ↓ '                \
           ' ↑ ↑ ↑ → → ↓ ↓ ↓ ↓ '                    \
           ' ↑ ↑ ↑ ↑ ← ↓ ↓ ↓ ↓ '                      >    INSPECTION OBJECTS
           ' ↑ ↑ ↑ ← ← ← ↓ ↓ ↓ '                    /
           ' ↑ ↑ ← ← ← ← ← ↓ ↓ '                /
           ' ↑ ← ← ← ← ← ← ← ↓ '            /
           ' ← ← ← ← ← ← ← ← ← '        /
           + - - - - - - - - - +    /
        */
#define MX (_world->irs->matrix)
        Matrix::matrix_key_t camKey = Matrix::matrix_get_key(params.camera_position);
        // unordered_map<int,... <Transform*>>
        for(auto &layer : MX)
        {
            // Render Objects
            storm_cast_eq_all(
                camKey,
                params.edges,
                [&](const Vec2Int &candidate)
                {
                    // unordered_map<Vec2Int,... <Transform*>>
                    const auto &layerObject = layer.second.find(candidate);
                    if(layerObject != std::end(layer.second))
                    {
                        for(Transform *transform_object : layerObject->second)
                        {
                            for(Component *component : transform_object->_owner->m_components)
                            {
                                Renderer *render_iobject;
                                // This object not render component, then continue and to next iterator
                                if(!component->_enable || (render_iobject = dynamic_cast<Renderer *>(component)) == nullptr)
                                {
                                    continue;
                                }

                                params.orders[render_iobject->_owner->m_zOrder].push_back(render_iobject);
                            }
                        }
                    }
                });
        }

        // orders
        for(auto layer = std::begin(params.orders); layer != std::end(params.orders); ++layer)
        {
            for(Renderer *renderRef : layer->second)
            {
                Transform *render_transform = renderRef->transform();
                memset(&(params.wrapper), 0, sizeof(params.wrapper));

                // draw
                SpriteRenderer *spriteRenderer;
                if((spriteRenderer = dynamic_cast<SpriteRenderer *>(renderRef)) == nullptr)
                    continue;

                // Render
                render_sprite_renderer(RenderCommand::PreRender, spriteRenderer, &params.wrapper);

                if(params.wrapper.texture)
                {
                    params.sourcePoint = render_transform->_position;
                    params.sourcePoint += spriteRenderer->get_offset();

                    params.wrapper.dst.w *= _world->irs->metricPixelsPerPoint.x; //_scale.x;
                    params.wrapper.dst.h *= _world->irs->metricPixelsPerPoint.y; //_scale.y;

                    // convert world to screen
                    // Horizontal
                    params.wrapper.dst.x += ((gscope.activeResolution.width - params.wrapper.dst.w) / 2.0f - (params.camera_position.x - params.sourcePoint.x) * _world->irs->metricPixelsPerPoint.x);
                    // Vertical
                    params.wrapper.dst.y += ((gscope.activeResolution.height - params.wrapper.dst.h) / 2.0f + (params.camera_position.y - params.sourcePoint.y) * _world->irs->metricPixelsPerPoint.y);

                    // draw to backbuffer
                    SDL_RenderCopyExF(gscope.renderer, params.wrapper.texture, reinterpret_cast<SDL_Rect *>(&(params.wrapper.src)), reinterpret_cast<SDL_FRect *>(&(params.wrapper.dst)), render_transform->_angle_, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);

                    // Send command post render
                    render_sprite_renderer(RenderCommand::PostRender, spriteRenderer, &params.wrapper);

                    ++(camera->res->culled);
                }
            }
            layer->second.clear();

            // Optimize Orders
            if(layer->second.capacity() > 1024)
            {
                layer->second.reserve(1024);
            }
        }

#undef MX

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

                        SDL_RenderCopyExF(renderer, wrapper.texture, reinterpret_cast<SDL_Rect*>(&wrapper.src),
           reinterpret_cast<SDL_FRect*>(&wrapper.dst), lightSource->transform()->_angle_ - transform()->_angle_,
           nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
                    }
                }
        */
        if(camera->visibleBorders)
        {
            float offset = 25 * std::max(1 - Time::deltaTime(), 0.1f);
            float height = 200 * Time::deltaTime();

            params.wrapper.dst.x = ((gscope.activeResolution.width) / 2.0f);
            params.wrapper.dst.y = ((gscope.activeResolution.height) / 2.0f);

            SDL_SetRenderDrawColor(gscope.renderer, 0, 255, 0, 25);

            // Center dot
            SDL_RenderDrawPointF(gscope.renderer, params.wrapper.dst.x, params.wrapper.dst.y);
            SDL_RenderDrawPointF(gscope.renderer, params.wrapper.dst.x - offset, params.wrapper.dst.y);
            SDL_RenderDrawPointF(gscope.renderer, params.wrapper.dst.x + offset, params.wrapper.dst.y);
            SDL_RenderDrawPointF(gscope.renderer, params.wrapper.dst.x, params.wrapper.dst.y - offset);
            SDL_RenderDrawPointF(gscope.renderer, params.wrapper.dst.x, params.wrapper.dst.y + offset);

            // borders
            RenderUtility::DrawLine(Vec2(offset, offset), Vec2(offset + height, offset));
            RenderUtility::DrawLine(Vec2(gscope.activeResolution.width - offset, offset), Vec2(gscope.activeResolution.width - offset - height, offset));
            RenderUtility::DrawLine(Vec2(offset, gscope.activeResolution.height - offset), Vec2(offset + height, gscope.activeResolution.height - offset));
            RenderUtility::DrawLine(Vec2(gscope.activeResolution.width - offset, gscope.activeResolution.height - offset), Vec2(gscope.activeResolution.width - offset - height, gscope.activeResolution.height - offset));

            RenderUtility::DrawLine(Vec2(offset, 1 + offset), Vec2(offset, offset + height));
            RenderUtility::DrawLine(Vec2(gscope.activeResolution.width - offset, 1 + offset), Vec2(gscope.activeResolution.width - offset, offset + height));
            RenderUtility::DrawLine(Vec2(offset, gscope.activeResolution.height - 1 - offset), Vec2(offset, gscope.activeResolution.height - offset - height));
            RenderUtility::DrawLine(Vec2(gscope.activeResolution.width - offset, gscope.activeResolution.height - 1 - offset), Vec2(gscope.activeResolution.width - offset, gscope.activeResolution.height - offset - height));
        }

        if(camera->visibleObjects || camera->visibleNames)
        {
            /* for(const auto &layer : (*std::get<0>(filter)))
                 for(Renderer *face : layer.second)
                 {

                     Vec2 p = face->transform()->position() + face->get_offset();
                     if(camera->visibleObjects)
                     {
                         Rect factSz = face->get_relative_size();
                         Vec2 sz = Vec2::Scale(face->get_size(), Vec2(factSz.getWH()) / pixelsPerPoint);
                         Gizmos::SetColor(Color::blue);
                         Gizmos::DrawRectangleRounded(p, sz.x, sz.y, 10);
                         Gizmos::SetColor(Color::black);
                         Gizmos::DrawPosition(p, 0.2f);
                     }
                     if(camera->visibleNames)
                     {
                         Gizmos::SetColor(Color::white);
                         Gizmos::DrawText(p, face->gameObject()->m_name);
                     }
                 }*/
        }
    }
} // namespace RoninEngine::Runtime
