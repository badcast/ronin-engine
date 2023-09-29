#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    void native_render_2D(Camera2D *camera);
    // void native_render_3D(Camera3D *camera);

    inline bool area_cast(Renderer *target, const Vec2Int &wpLeftTop, const Vec2Int &wpRightBottom)
    {
        Vec2 rSz = target->get_size();
        Vec2 pos = target->transform()->position();
        return (pos.x + rSz.x >= wpLeftTop.x && pos.x - rSz.x <= wpRightBottom.x) &&
            (pos.y - rSz.y <= wpLeftTop.y && pos.y + rSz.y >= wpRightBottom.y);
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

    Camera2D::Camera2D(const std::string &name)
        : Camera(DESCRIBE_AS_ONLY_NAME(Camera2D)), scale(Vec2::one), visibleBorders(false), visibleGrids(false), visibleObjects(false)
    {
        DESCRIBE_AS_MAIN(Camera2D);
        distanceEvcall = 1;
    }

    Camera2D::Camera2D(const Camera2D &other)
        : Camera(other.m_name),
          visibleBorders(other.visibleBorders),
          visibleGrids(other.visibleGrids),
          visibleObjects(other.visibleObjects),
          scale(other.scale)
    {
    }

    void native_render_2D(Camera2D *camera)
    {
        struct
        {
            Rendering wrapper;
            Transform *root_transform;
            Vec2 sourcePoint, camera_position;
            Vec2Int wpLeftTop;
            Vec2Int wpRightBottom;
            int edges;
        } stack;
        camera->camera_resources->culled = 0;

        stack.camera_position = camera->transform()->position();
        stack.root_transform = World::self()->irs->main_object->transform();
        stack.wpLeftTop = Vec2::RoundToInt(Camera::ScreenToWorldPoint(Vec2::zero));
        stack.wpRightBottom = Vec2::RoundToInt(Camera::ScreenToWorldPoint(Vec2(active_resolution.width, active_resolution.height)));
        stack.edges =
            Math::Number(Math::Max(stack.wpRightBottom.x - stack.camera_position.x, stack.wpRightBottom.y - stack.camera_position.y)) + 5 +
            camera->distanceEvcall;

        Color prevColor = Gizmos::GetColor();
        Gizmos::SetColor(0xffc4c4c4);

        if(camera->visibleGrids)
        {
            Gizmos::Draw2DWorldSpace(Vec2::zero);
            Gizmos::DrawPosition(stack.camera_position, maxWorldScalar);
        }

        // scale.x = Mathf::Min(Mathf::Max(scale.x, 0.f), 1000.f);
        // scale.y = Mathf::Min(Mathf::Max(scale.y, 0.f), 1000.f);
        //_scale = scale*squarePerPixels;
        SDL_RenderSetScale(RoninEngine::renderer, camera->scale.x, camera->scale.y);

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
#define MX (switched_world->irs->matrix)
        MatrixKey camKey = Matrix::matrix_get_key(stack.camera_position);
        // unordered_map<int,... <Transform*>>
        for(auto layer : MX)
        {
            storm_cast_eq_edges(
                camKey,
                stack.edges,
                [&](const Vec2Int &candidate)
                {
                    // unordered_map<Vec2Int,... <Transform*>>
                    auto layerObject = layer.second.find(candidate);
                    if(layerObject != std::end(layer.second))
                    {
                        for(Transform *transform_object : layerObject->second)
                        {break;
                            for(Component *component : transform_object->_owner->m_components)
                            {
                                Renderer *render_iobject;
                                // This object not render component, then continue and to next iterator
                                if((render_iobject = dynamic_cast<Renderer *>(component)) == nullptr)
                                {
                                    continue;
                                }

                                ++(camera->camera_resources->culled);

                                Transform *render_transform = render_iobject->transform();
                                memset(&(stack.wrapper), 0, sizeof(stack.wrapper));

                                // draw
                                render_iobject->render(&(stack.wrapper));

                                if(stack.wrapper.texture)
                                {
                                    Transform *render_parent = render_transform->m_parent;
                                    if(render_parent == stack.root_transform)
                                    {
                                        stack.sourcePoint = render_transform->_position;
                                    }
                                    else
                                    {
                                        // local position is direction
                                        stack.sourcePoint = Vec2::RotateAround(
                                            render_parent->_position,
                                            render_transform->localPosition(),
                                            render_parent->angle() * Math::deg2rad);
                                    }

                                    stack.wrapper.dst.w *= pixelsPerPoint; //_scale.x;
                                    stack.wrapper.dst.h *= pixelsPerPoint; //_scale.y;

                                    Vec2 arranged = stack.wrapper.dst.getXY();
                                    if(arranged != Vec2::zero)
                                        arranged =
                                            Vec2::RotateAround(stack.sourcePoint, arranged, render_transform->angle() * Math::deg2rad);

                                    stack.sourcePoint += render_iobject->get_offset();

                                    // convert world to screen

                                    // Положение по горизонтале
                                    stack.wrapper.dst.x = arranged.x +
                                        ((active_resolution.width - stack.wrapper.dst.w) / 2.0f -
                                         (stack.camera_position.x - stack.sourcePoint.x) * pixelsPerPoint);
                                    // Положение по вертикале
                                    stack.wrapper.dst.y = arranged.y +
                                        ((active_resolution.height - stack.wrapper.dst.h) / 2.0f +
                                         (stack.camera_position.y - stack.sourcePoint.y) * pixelsPerPoint);
                                    // draw to backbuffer
                                    SDL_RenderCopyExF(
                                        RoninEngine::renderer,
                                        stack.wrapper.texture,
                                        (SDL_Rect *) &(stack.wrapper.src),
                                        reinterpret_cast<SDL_FRect *>(&(stack.wrapper.dst)),
                                        render_transform->angle(),
                                        nullptr,
                                        SDL_RendererFlip::SDL_FLIP_NONE);
                                }
                            }
                        }
                    }
                });
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
            float offset = 25 * std::max(1 - TimeEngine::deltaTime(), 0.1f);
            float height = 200 * TimeEngine::deltaTime();

            stack.wrapper.dst.x = ((active_resolution.width) / 2.0f);
            stack.wrapper.dst.y = ((active_resolution.height) / 2.0f);

            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 25);

            // Center dot
            SDL_RenderDrawPointF(renderer, stack.wrapper.dst.x, stack.wrapper.dst.y);
            SDL_RenderDrawPointF(renderer, stack.wrapper.dst.x - offset, stack.wrapper.dst.y);
            SDL_RenderDrawPointF(renderer, stack.wrapper.dst.x + offset, stack.wrapper.dst.y);
            SDL_RenderDrawPointF(renderer, stack.wrapper.dst.x, stack.wrapper.dst.y - offset);
            SDL_RenderDrawPointF(renderer, stack.wrapper.dst.x, stack.wrapper.dst.y + offset);

            // borders
            Gizmos::DrawLine(Vec2(offset, offset), Vec2(offset + height, offset));
            Gizmos::DrawLine(Vec2(active_resolution.width - offset, offset), Vec2(active_resolution.width - offset - height, offset));
            Gizmos::DrawLine(Vec2(offset, active_resolution.height - offset), Vec2(offset + height, active_resolution.height - offset));
            Gizmos::DrawLine(
                Vec2(active_resolution.width - offset, active_resolution.height - offset),
                Vec2(active_resolution.width - offset - height, active_resolution.height - offset));

            Gizmos::DrawLine(Vec2(offset, 1 + offset), Vec2(offset, offset + height));
            Gizmos::DrawLine(Vec2(active_resolution.width - offset, 1 + offset), Vec2(active_resolution.width - offset, offset + height));
            Gizmos::DrawLine(Vec2(offset, active_resolution.height - 1 - offset), Vec2(offset, active_resolution.height - offset - height));
            Gizmos::DrawLine(
                Vec2(active_resolution.width - offset, active_resolution.height - 1 - offset),
                Vec2(active_resolution.width - offset, active_resolution.height - offset - height));
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

        Gizmos::SetColor(prevColor);
    }
} // namespace RoninEngine::Runtime
