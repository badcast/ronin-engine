#include "ronin.h"
#include "ronin_matrix.h"
#include "ronin_render_cache.h"

namespace RoninEngine::Runtime
{

    enum RenderFuncClass
    {
        REND_UNDEFINED = 0,
        REND_SPRITE,
        REND_TERRAIN2D
    };

    void native_render_2D(Camera2D *camera);
    void native_render_3D(Camera *camera);
    void render_sprite_renderer(RenderCommand command, Renderer *object, Rendering *rendering);
    void render_terrain2d(RenderCommand command, Renderer *object, Rendering *rendering);

    struct
    {
        Rendering wrapper;
        Transform *root_transform;
        Vec2 sourcePoint, camera_position;
        Vec2Int wpLeftTop;
        Vec2Int wpRightBottom;
        std::map<int, std::vector<Renderer *>> orders;
        std::vector<std::function<void(RenderCommand, Renderer *, Rendering *)>> renderFunctions {render_sprite_renderer, render_terrain2d};
        int edges;
    } params;

    template <>
    int render_getclass<SpriteRenderer>()
    {
        return REND_SPRITE;
    }
    template <>
    int render_getclass<Terrain2D>()
    {
        return REND_TERRAIN2D;
    }

    constexpr std::function<void(RenderCommand, Renderer *, Rendering *)> render_getfunc(int _class)
    {
        if(_class < 1 || params.renderFunctions.size() < _class)
        {
            return nullptr;
        }
        return params.renderFunctions[--_class];
    }

    inline bool area_cast(Renderer *target, const Vec2Int &wpLeftTop, const Vec2Int &wpRightBottom)
    {
        Vec2 rSz = target->getSize();
        Vec2 pos = target->transform()->position();
        return (pos.x + rSz.x >= wpLeftTop.x && pos.x - rSz.x <= wpRightBottom.x) && (pos.y - rSz.y <= wpLeftTop.y && pos.y + rSz.y >= wpRightBottom.y);
    }

    /////////////////
    /// CAMERA 2D ///
    /////////////////
    void native_render_2D(Camera2D *camera)
    {
        camera->res->culled = 0;

        params.camera_position = camera->transform()->position();
        params.root_transform = World::GetCurrentWorld()->irs->mainObject->transform().ptr_;
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
        Matrix::matrix_key_t camKey = Matrix::matrix_get_key(params.camera_position);
        // unordered_map<int,... <Transform*>>
        for(auto &layer : currentWorld->irs->matrix)
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
                            for(ComponentRef &component : transform_object->_owner->m_components)
                            {
                                Renderer *render_iobject;
                                // This object not render component, then continue and to next iterator
                                if(!component->_enable || (render_iobject = dynamic_cast<Renderer *>(component.ptr_)) == nullptr)
                                {
                                    continue;
                                }

                                params.orders[render_iobject->_owner->m_zOrder].push_back(render_iobject);
                            }
                        }
                    }
                });
        }

        for(std::map<int, std::vector<Renderer *>>::iterator layer = std::begin(params.orders); layer != std::end(params.orders); ++layer)
        {
            for(Renderer *renderRef : layer->second)
            {
                Transform *render_transform = renderRef->transform().ptr_;
                memset(&(params.wrapper), 0, sizeof(params.wrapper));
                // Send command pre render
                render_getfunc(renderRef->_class)(RenderCommand::PreRender, renderRef, &params.wrapper);
                if(params.wrapper.texture)
                {
                    params.sourcePoint = render_transform->_position;
                    params.sourcePoint += renderRef->get_offset();

                    params.wrapper.dst.w *= currentWorld->irs->metricPixelsPerPoint.x * camera->res->scale.x;
                    params.wrapper.dst.h *= currentWorld->irs->metricPixelsPerPoint.y * camera->res->scale.y;

                           // convert world to screen

                           // Horizontal
                    params.wrapper.dst.x += ((gscope.activeResolution.width - params.wrapper.dst.w) / 2.0f - (params.camera_position.x - params.sourcePoint.x) * currentWorld->irs->metricPixelsPerPoint.x * camera->res->scale.x);
                    // Vertical
                    params.wrapper.dst.y += ((gscope.activeResolution.height - params.wrapper.dst.h) / 2.0f + (params.camera_position.y - params.sourcePoint.y) * currentWorld->irs->metricPixelsPerPoint.y * camera->res->scale.y);

                           // draw to backbuffer
                    SDL_RenderCopyExF(gscope.renderer, params.wrapper.texture, reinterpret_cast<SDL_Rect *>(&(params.wrapper.src)), reinterpret_cast<SDL_FRect *>(&(params.wrapper.dst)), render_transform->_angle_, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);

                    ++(camera->res->culled);
                }
                // Send command post render
                render_getfunc(renderRef->_class)(RenderCommand::PostRender, renderRef, &params.wrapper);
            }
            layer->second.clear();
        }

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

    /////////////////
    /// CAMERA 3D ///
    /////////////////
    // TODO: Make Camera 3D

    ///////////////////////
    /// SPRITE RENDERER ///
    ///////////////////////
    void render_sprite_renderer(RenderCommand command, Renderer *object, Rendering *rendering)
    {
        SpriteRenderer *target = static_cast<SpriteRenderer *>(object);
        if(target->sprite == nullptr)
            return;

        switch(command)
        {
            case RenderCommand::PreRender:
            {
                if(target->save_texture == nullptr)
                {
                    std::uint16_t x, y;
                    SDL_Rect dest;

                    switch(target->renderType)
                    {
                        case SpriteRenderType::Simple:

                            target->save_texture = render_cache_texture(target->sprite.ptr_);
                            rendering->src = target->sprite->m_rect;
                            rendering->dst.w = target->sprite->m_rect.w;
                            rendering->dst.h = target->sprite->m_rect.h;

                            break;
                            target->save_texture = SDL_CreateTextureFromSurface(gscope.renderer, target->sprite->surface);
                            rendering->src.w = target->sprite->width();
                            rendering->src.h = target->sprite->height();
                            rendering->dst.w = target->sprite->width();
                            rendering->dst.h = target->sprite->height();
                            switch(target->renderPresentMode)
                            {
                                    // render as fixed (Resize mode)
                                    // case SpriteRenderPresentMode::Fixed:
                                    //      rendering->dst.w = sprite->width() * abs(target->m_size.x) / pixelsPerPoint;
                                    //      rendering->dst.h = sprite->height() * abs(target->m_size.y) / pixelsPerPoint;
                                    // break;

                                           // render as cut
                                case SpriteRenderPresentMode::Place:
                                    rendering->src.w *= target->m_size.x;
                                    rendering->src.h *= target->m_size.y;
                                    break;
                            }
                            break;
                        case SpriteRenderType::Tile:
                        {
                            // TODO: Use render_texture_extension for tiles

                            rendering->src.w = abs(target->m_size.x) * target->sprite->width();
                            rendering->src.h = abs(target->m_size.y) * target->sprite->width();
                            rendering->dst.w = target->sprite->width() * abs(target->m_size.x) / currentWorld->irs->metricPixelsPerPoint.x;
                            rendering->dst.h = target->sprite->height() * abs(target->m_size.y) / currentWorld->irs->metricPixelsPerPoint.y;

                                   // generate tiles
                            target->save_texture = SDL_CreateTexture(
                                                                      // renderer, sdl_default_pixelformat, SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET, rendering->src.w,
                                                                      // rendering->src.h);
                                gscope.renderer,
                                defaultPixelFormat,
                                SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET,
                                1024,
                                1024);
                            if(target->save_texture == nullptr)
                                RoninSimulator::ShowMessageFail("Texture create fail");

                            SDL_SetRenderTarget(gscope.renderer, target->save_texture);

                            dest.w = target->sprite->width();
                            dest.h = target->sprite->height();

                            rendering->src.x = rendering->src.w / dest.w;
                            rendering->src.y = rendering->src.h / dest.h;

                            SDL_Texture *temp_texture = SDL_CreateTextureFromSurface(gscope.renderer, target->sprite->surface);

                                   // render tile
                            switch(target->renderPresentMode)
                            {
                                case SpriteRenderPresentMode::Fixed:
                                {
                                    for(x = 0; x < rendering->src.x; ++x)
                                    {
                                        for(y = 0; y < rendering->src.y; ++y)
                                        {
                                            dest.x = x * dest.w;
                                            dest.y = y * dest.h;
                                            SDL_RenderCopy(gscope.renderer, temp_texture, (SDL_Rect *) &target->sprite->m_rect, (SDL_Rect *) &dest);
                                        }
                                    }
                                    break;
                                }
                                case SpriteRenderPresentMode::Place:
                                {
                                    for(x = 0; x < rendering->src.x; ++x)
                                    {
                                        for(y = 0; y < rendering->src.y; ++y)
                                        {
                                            dest.x = x * dest.w;
                                            dest.y = y * dest.h;

                                            SDL_RenderCopy(gscope.renderer, temp_texture, (SDL_Rect *) &target->sprite->m_rect, (SDL_Rect *) &dest);
                                        }
                                    }
                                    // place remained
                                    for(x = 0, dest.y = rendering->src.h / dest.h * dest.h, dest.h = target->m_size.y - dest.y; x < rendering->src.x; ++x)
                                    {
                                        dest.x = x * dest.w;
                                        SDL_RenderCopy(gscope.renderer, temp_texture, (SDL_Rect *) &target->sprite->m_rect, (SDL_Rect *) &dest);
                                    }
                                    ++rendering->src.y;
                                    for(y = 0, dest.x = rendering->src.w / dest.w * dest.w, dest.h = target->sprite->height(), dest.w = target->m_size.x - dest.x; y < rendering->src.y; ++y)
                                    {
                                        dest.y = y * dest.h;
                                        SDL_RenderCopy(gscope.renderer, temp_texture, (SDL_Rect *) &target->sprite->m_rect, (SDL_Rect *) &dest);
                                    }

                                    break;
                                }
                            }
                            SDL_DestroyTexture(temp_texture);
                            SDL_SetRenderTarget(gscope.renderer, nullptr);
                            break;
                        }
                    }
                    target->save_src = rendering->src;
                    target->save_dst = rendering->dst;

                    SDL_GetTextureColorMod(target->save_texture, &target->prevColorTexture.r, &target->prevColorTexture.g, &target->prevColorTexture.b);
                    SDL_GetTextureAlphaMod(target->save_texture, &target->prevColorTexture.a);
                }
                else
                {
                    rendering->src = target->save_src;
                    rendering->dst = target->save_dst;
                }

                if(target->save_texture)
                {
                    SDL_SetTextureColorMod(target->save_texture, target->color.r, target->color.g, target->color.b);
                    SDL_SetTextureAlphaMod(target->save_texture, target->color.a);
                }

                rendering->dst.w = (rendering->dst.w * target->m_size.x / currentWorld->irs->metricPixelsPerPoint.x) * ((static_cast<int>(target->flip) & static_cast<int>(SpriteRenderFlip::FlipHorizontal)) ? -1 : 1);
                rendering->dst.h = (rendering->dst.h * target->m_size.y / currentWorld->irs->metricPixelsPerPoint.y) * ((static_cast<int>(target->flip) & static_cast<int>(SpriteRenderFlip::FlipVertical)) ? -1 : 1);

                rendering->texture = target->save_texture;
            }
            break;
            case RenderCommand::PostRender:
                if(target->save_texture == nullptr)
                    break;
                SDL_SetTextureColorMod(target->save_texture, target->prevColorTexture.r, target->prevColorTexture.g, target->prevColorTexture.b);
                SDL_SetTextureAlphaMod(target->save_texture, target->prevColorTexture.a);
                break;
            default:;
        }
    }

           ///////////////////////////
           /// TERRAIN 2D RENDERER ///
           ///////////////////////////
    void render_terrain2d(RenderCommand command, Renderer *object, Rendering *rendering)
    {
        Terrain2D *target = static_cast<Terrain2D *>(object);
        switch(command)
        {
            case RenderCommand::PreRender:

                break;
            case RenderCommand::PostRender:

                break;
            default:;
        }
    }

} // namespace RoninEngine::Runtime
