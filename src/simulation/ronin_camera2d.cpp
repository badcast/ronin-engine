#include "ronin.h"
#include "omp.h"

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

    Camera2D::Camera2D(const Camera2D& other)
        : Camera(other.m_name)
        , visibleBorders(other.visibleBorders)
        , visibleGrids(other.visibleGrids)
        , visibleObjects(other.visibleObjects)
        , scale(other.scale)
    {
    }

    void Camera2D::render(SDL_Renderer* renderer, Rect rect, GameObject* root)
    {
        Rendering wrapper;
        Vec2* point;
        Vec2 sourcePoint;
        Color prevColor = Gizmos::get_color();
        SDL_mutex* m = SDL_CreateMutex();
        Gizmos::set_color(0xffc4c4c4);
        if (visibleGrids) {
            Gizmos::draw_2D_world_space(Vec2::zero);
            Gizmos::draw_position(transform()->position(), maxWorldScalar);
        }
        // get from matrix selection
        std::tuple<std::map<int, std::set<Renderer*>>*, std::set<Light*>*> filter = matrix_select();

        // scale.x = Mathf::Min(Mathf::Max(scale.x, -10.f), 10.f);
        // scale.y = Mathf::Min(Mathf::Max(scale.y, -10.f), 10.f);
        //_scale = scale*squarePerPixels;
        SDL_RenderSetScale(renderer, scale.x, scale.y);

        // Render Objects
        for (auto& layer : *(std::get<0>(filter))) {
            std::vector<Renderer*> robject;
            robject.reserve(layer.second.size());
            for (auto ib = layer.second.begin(); ib != layer.second.end(); ++ib) {
                robject.emplace_back(*ib);
            }
            int size = robject.size();
            // #pragma omp parallel for private(sourcePoint) private(wrapper) private(point)
            for (std::size_t pointer = 0; pointer < size; ++pointer) {
                Renderer* renderSource = robject[pointer];
                memset(&wrapper, 0, sizeof(wrapper));
                wrapper.renderer = renderer;
                SDL_LockMutex(m);
                renderSource->render(&wrapper); // draw
                SDL_UnlockMutex(m);
                if (wrapper.texture) {
                    // FIXME: point (transform()->position) ? wtf?
                    Vec2 point = transform()->p;
                    Transform* rTrans = renderSource->transform();

                    if (rTrans->m_parent && renderSource->transform()->m_parent != get_root(World::self())) {
                        Vec2 direction = rTrans->p;
                        sourcePoint = Vec2::rotate_around(rTrans->m_parent->position(), direction, rTrans->angle() * Math::deg2rad);
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
                        arranged = Vec2::rotate_around(sourcePoint, arranged, renderSource->transform()->angle() * Math::deg2rad);

                    sourcePoint += renderSource->get_offset();

                    // convert world to screen

                    // Положение по горизонтале
                    wrapper.dst.x = arranged.x + ((rect.w - wrapper.dst.w) / 2.0f - (point.x - sourcePoint.x) * pixelsPerPoint);
                    // Положение по вертикале
                    wrapper.dst.y = arranged.y + ((rect.h - wrapper.dst.h) / 2.0f + (point.y - sourcePoint.y) * pixelsPerPoint);

                    SDL_LockMutex(m);
                    // native renderer component
                    SDL_RenderCopyExF(renderer, wrapper.texture, (SDL_Rect*)&wrapper.src, reinterpret_cast<SDL_FRect*>(&wrapper.dst), renderSource->transform()->angle(), nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
                    SDL_DestroyTexture(wrapper.texture);
                    SDL_UnlockMutex(m);
                }
            }
        }
        SDL_DestroyMutex(m);
        // Render Lights
        for (auto lightSource : *std::get<1>(filter)) {
            // drawing
            // clear
            memset(&wrapper, 0, sizeof wrapper);
            wrapper.renderer = renderer;

            lightSource->get_light_source(&wrapper); // draw

            if (wrapper.texture) {
                point = &transform()->p;
                sourcePoint = lightSource->transform()->position();

                wrapper.dst.w *= pixelsPerPoint;

                wrapper.dst.h *= pixelsPerPoint;

                // h
                wrapper.dst.x += ((rect.w - wrapper.dst.w) / 2.0f - (point->x - sourcePoint.x) * pixelsPerPoint);
                // v
                wrapper.dst.y += ((rect.h - wrapper.dst.h) / 2.0f + (point->y - sourcePoint.y) * pixelsPerPoint);

                SDL_RenderCopyExF(renderer, wrapper.texture, reinterpret_cast<SDL_Rect*>(&wrapper.src), reinterpret_cast<SDL_FRect*>(&wrapper.dst), lightSource->transform()->_angle_ - transform()->_angle_, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
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
            Gizmos::draw_line(Vec2(rect.x + offset, rect.y + offset), Vec2(rect.x + offset + height, rect.y + offset));
            Gizmos::draw_line(Vec2(rect.w - offset, rect.y + offset), Vec2(rect.w - offset - height, rect.y + offset));
            Gizmos::draw_line(Vec2(rect.x + offset, rect.h - offset), Vec2(rect.x + offset + height, rect.h - offset));
            Gizmos::draw_line(Vec2(rect.w - offset, rect.h - offset), Vec2(rect.w - offset - height, rect.h - offset));

            Gizmos::draw_line(Vec2(rect.x + offset, rect.y + 1 + offset), Vec2(rect.x + offset, rect.y + offset + height));
            Gizmos::draw_line(Vec2(rect.w - offset, rect.y + 1 + offset), Vec2(rect.w - offset, rect.y + offset + height));
            Gizmos::draw_line(Vec2(rect.x + offset, rect.h - 1 - offset), Vec2(rect.x + offset, rect.h - offset - height));
            Gizmos::draw_line(Vec2(rect.w - offset, rect.h - 1 - offset), Vec2(rect.w - offset, rect.h - offset - height));
        }

        if (visibleObjects || visibleNames) {
            for (const auto& layer : (*std::get<0>(filter)))
                for (Renderer* face : layer.second) {

                    Vec2 p = face->transform()->position() + face->get_offset();
                    if (visibleObjects) {
                        Rect factSz = face->get_relative_size();
                        Vec2 sz = Vec2::scale(face->get_size(), Vec2(factSz.getWH()) / pixelsPerPoint);
                        Gizmos::set_color(Color::blue);
                        Gizmos::draw_rectangle_rounded(p, sz.x, sz.y, 10);
                        Gizmos::set_color(Color::black);
                        Gizmos::draw_position(p, 0.2f);
                    }
                    if (visibleNames) {
                        Gizmos::set_color(Color::white);
                        Gizmos::draw_text(p, face->game_object()->m_name);
                    }
                }
        }

        Gizmos::set_color(prevColor);
    }
} // namespace RoninEngine::Runtime
