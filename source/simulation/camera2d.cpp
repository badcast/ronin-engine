#include "ronin.h"

// TODO: how to create shadow ? for all sprite renderer component
using namespace RoninEngine;

namespace RoninEngine::Runtime
{
    Camera2D::Camera2D()
        : Camera(DESCRIBE_AS_MAIN_OFF(Camera2D))
        , scale(Vec2::one)
    {
        this->visibleBorders = false;
        this->visibleGrids = false;
        this->visibleObjects = false;
    }

    Camera2D::Camera2D(const std::string& name)
        : Camera(DESCRIBE_AS_ONLY_NAME(Camera2D))
    {
        DESCRIBE_AS_MAIN(Camera2D);
    }

    RoninEngine::Runtime::Camera2D::Camera2D(const Camera2D& proto)
        : Camera(proto.m_name)
        , visibleBorders(proto.visibleBorders)
        , visibleGrids(proto.visibleGrids)
        , visibleObjects(proto.visibleObjects)
        , scale(proto.scale)
    {
    }

    void Camera2D::render(SDL_Renderer* renderer, Rect rect, GameObject* root)
    {
        Rendering wrapper;
        Vec2* point;
        Vec2 sourcePoint;
        // Vec2 _scale;

        Color prevColor = Gizmos::getColor();

        Gizmos::setColor(0xffc4c4c4);

        if (visibleGrids) {
            Gizmos::Draw2DWorldSpace(Vec2::zero);
            Gizmos::DrawPosition(transform()->position(), maxWorldScalar);
        }
        // get from matrix selection
        auto filter = matrix_select();

        // scale.x = Mathf::Min(Mathf::Max(scale.x, -10.f), 10.f);
        // scale.y = Mathf::Min(Mathf::Max(scale.y, -10.f), 10.f);
        //_scale = scale*squarePerPixels;
        SDL_RenderSetScale(renderer, scale.x, scale.y);

        // Render Objects
        for (const auto& layer : *(std::get<0>(filter)))
            for (auto renderSource : layer.second) {
                // drawing
                // clear
                wrapper = {};
                wrapper.renderer = renderer;

                renderSource->render(&wrapper); // draw
                if (wrapper.texture) {
                    // FIXME: point (transform()->position) ? wtf?
                    Vec2 point = transform()->p;
                    Transform* rTrans = renderSource->transform();

                    if (rTrans->m_parent && renderSource->transform()->m_parent != Level::self()->main_object->transform()) {
                        Vec2 direction = rTrans->p;
                        sourcePoint = Vec2::RotateAround(rTrans->m_parent->position(), direction, rTrans->angle() * Math::Deg2Rad);
                        //                rTrans->localPosition(
                        //                    Vec2::Max(direction, Vec2::RotateAround(Vec2::zero, direction, rTrans->angle() *
                        //                    Mathf::Deg2Rad)));

                    } else {
                        sourcePoint = rTrans->position();
                    }

                    wrapper.dst.w *= pixelsPerPoint; //_scale.x;
                    wrapper.dst.h *= pixelsPerPoint; //_scale.y;

                    Vec2 arranged(wrapper.dst.x, wrapper.dst.y);
                    if (arranged != Vec2::zero)
                        arranged = Vec2::RotateAround(sourcePoint, arranged, renderSource->transform()->angle() * Math::Deg2Rad);

                    sourcePoint += renderSource->get_offset();

                    // convert world to screen

                    // Положение по горизонтале
                    wrapper.dst.x = arranged.x + ((rect.w - wrapper.dst.w) / 2.0f - (point.x - sourcePoint.x) * pixelsPerPoint);
                    // Положение по вертикале
                    wrapper.dst.y = arranged.y + ((rect.h - wrapper.dst.h) / 2.0f + (point.y - sourcePoint.y) * pixelsPerPoint);

                    // native renderer component
                    SDL_RenderCopyExF(renderer, wrapper.texture, (SDL_Rect*)&wrapper.src, reinterpret_cast<SDL_FRect*>(&wrapper.dst), renderSource->transform()->angle(), nullptr, SDL_RendererFlip::SDL_FLIP_NONE);

                    SDL_DestroyTexture(wrapper.texture);
                }
            }
        // Render Lights
        for (auto lightSource : *std::get<1>(filter)) {
            // drawing
            // clear
            memset(&wrapper, 0, sizeof wrapper);
            wrapper.renderer = renderer;

            lightSource->GetLightSource(&wrapper); // draw

            if (wrapper.texture) {
                point = &transform()->p;
                sourcePoint = lightSource->transform()->position();

                wrapper.dst.w *= pixelsPerPoint;

                wrapper.dst.h *= pixelsPerPoint;

                // h
                wrapper.dst.x += ((rect.w - wrapper.dst.w) / 2.0f - (point->x - sourcePoint.x) * pixelsPerPoint);
                // v
                wrapper.dst.y += ((rect.h - wrapper.dst.h) / 2.0f + (point->y - sourcePoint.y) * pixelsPerPoint);

                SDL_RenderCopyExF(renderer, wrapper.texture, reinterpret_cast<SDL_Rect*>(&wrapper.src), reinterpret_cast<SDL_FRect*>(&wrapper.dst), lightSource->transform()->_angle - transform()->_angle, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
            }
        }

        if (visibleBorders) {
            float offset = 25 * std::max(1 - TimeEngine::deltaTime(), 0.5f);
            float height = 200 * TimeEngine::deltaTime();

            wrapper.dst.x = ((rect.w) / 2.0f);
            wrapper.dst.y = ((rect.h) / 2.0f);

            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 25);

            // Center dot
            SDL_RenderDrawPointF(renderer, wrapper.dst.x, wrapper.dst.y);
            SDL_RenderDrawPointF(renderer, wrapper.dst.x - offset, wrapper.dst.y);
            SDL_RenderDrawPointF(renderer, wrapper.dst.x + offset, wrapper.dst.y);
            SDL_RenderDrawPointF(renderer, wrapper.dst.x, wrapper.dst.y - offset);
            SDL_RenderDrawPointF(renderer, wrapper.dst.x, wrapper.dst.y + offset);

            // borders
            Gizmos::DrawLine(Vec2(rect.x + offset, rect.y + offset), Vec2(rect.x + offset + height, rect.y + offset));
            Gizmos::DrawLine(Vec2(rect.w - offset, rect.y + offset), Vec2(rect.w - offset - height, rect.y + offset));
            Gizmos::DrawLine(Vec2(rect.x + offset, rect.h - offset), Vec2(rect.x + offset + height, rect.h - offset));
            Gizmos::DrawLine(Vec2(rect.w - offset, rect.h - offset), Vec2(rect.w - offset - height, rect.h - offset));

            Gizmos::DrawLine(Vec2(rect.x + offset, rect.y + 1 + offset), Vec2(rect.x + offset, rect.y + offset + height));
            Gizmos::DrawLine(Vec2(rect.w - offset, rect.y + 1 + offset), Vec2(rect.w - offset, rect.y + offset + height));
            Gizmos::DrawLine(Vec2(rect.x + offset, rect.h - 1 - offset), Vec2(rect.x + offset, rect.h - offset - height));
            Gizmos::DrawLine(Vec2(rect.w - offset, rect.h - 1 - offset), Vec2(rect.w - offset, rect.h - offset - height));
        }

        if (visibleObjects || visibleNames) {
            for (const auto& layer : (*std::get<0>(filter)))
                for (Renderer* face : layer.second) {

                    Vec2 p = face->transform()->position() + face->get_offset();
                    if (visibleObjects) {
                        Rect factSz = face->get_relative_size();
                        Vec2 sz = Vec2::Scale(face->get_size(), Vec2(factSz.getWH()) / pixelsPerPoint);
                        Gizmos::setColor(Color::blue);
                        Gizmos::DrawRectangleRounded(p, sz.x, sz.y, 10);
                        Gizmos::setColor(Color::black);
                        Gizmos::DrawPosition(p, 0.2f);
                    }
                    if (visibleNames) {
                        Gizmos::setColor(Color::white);
                        Gizmos::DrawTextOnPosition(p, face->game_object()->m_name);
                    }
                }
        }

        Gizmos::setColor(prevColor);
    }
} // namespace RoninEngine::Runtime
