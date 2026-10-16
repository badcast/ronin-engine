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
        Vec2 wpLeftTop;
        Vec2 wpRightBottom;
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

    inline bool area_cast(Renderer *target, const Vec2 &wpLeftTop, const Vec2 &wpRightBottom)
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
        if(camera == nullptr || currentWorld == nullptr || currentWorld->irs == nullptr)
            return;

        camera->res->culled = 0;

        params.camera_position = camera->transform()->position();
        params.root_transform = World::GetCurrentWorld()->irs->mainObject->transform().ptr_;
        params.wpLeftTop = Camera::ScreenToWorldPoint(Vec2::zero);
        params.wpRightBottom = Camera::ScreenToWorldPoint(Vec2(gscope.activeResolution.width, gscope.activeResolution.height));
        params.edges = Math::Number(Math::Max(std::fabs(params.wpRightBottom.x - params.camera_position.x), std::fabs(params.wpRightBottom.y - params.camera_position.y))) + 5 + camera->distanceEvcall;

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

        float minX = std::min(params.wpLeftTop.x, params.wpRightBottom.x);
        float minY = std::min(params.wpLeftTop.y, params.wpRightBottom.y);
        float maxX = std::max(params.wpLeftTop.x, params.wpRightBottom.x);
        float maxY = std::max(params.wpLeftTop.y, params.wpRightBottom.y);
        float pad = static_cast<float>(std::max(0, static_cast<int>(camera->distanceEvcall)));
        Rectf camRect(minX - pad, minY - pad, (maxX - minX) + pad * 2.f, (maxY - minY) + pad * 2.f);

        std::set<Transform*> visibleTransforms;
        Matrix::matrix_get_intersects(camRect, visibleTransforms);

        std::vector<Light*> visibleLights;

        for (Transform* transform_object : visibleTransforms)
        {
            if (!transform_object || !transform_object->_owner)
                continue;

            for(ComponentRef &component : transform_object->_owner->m_components)
            {
                if (!component || !component->_enable)
                    continue;

                Renderer *render_iobject = component->get_renderer();
                if (render_iobject && render_iobject->_owner)
                {
                    params.orders[render_iobject->_owner->m_zOrder].push_back(render_iobject);
                }

                Light *light_object = component->get_light_component();
                if (light_object && light_object->_owner)
                {
                    visibleLights.push_back(light_object);
                }
            }
        }

        for(std::map<int, std::vector<Renderer *>>::iterator layer = std::begin(params.orders); layer != std::end(params.orders); ++layer)
        {
            for(Renderer *renderRef : layer->second)
            {
                Transform *render_transform = renderRef->transform().ptr_;
                memset(&(params.wrapper), 0, sizeof(params.wrapper));
                // Send command pre render
                auto renderFn = render_getfunc(renderRef->_class);
                if(!renderFn)
                    continue;
                renderFn(RenderCommand::PreRender, renderRef, &params.wrapper);
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
                    SDL_RenderCopyExF(gscope.renderer, params.wrapper.texture, reinterpret_cast<SDL_Rect *>(&(params.wrapper.src)), reinterpret_cast<SDL_FRect *>(&(params.wrapper.dst)), render_transform->_angle_, nullptr, params.wrapper.flip);

                    ++(camera->res->culled);
                }
                // Send command post render
                renderFn(RenderCommand::PostRender, renderRef, &params.wrapper);
            }
        }

        // Render Lights
        for (Light* lightSource : visibleLights)
        {
            memset(&(params.wrapper), 0, sizeof(params.wrapper));
            lightSource->get_light_source(&(params.wrapper));

            if (params.wrapper.texture)
            {
                params.sourcePoint = lightSource->transform()->position();

                params.wrapper.dst.w *= currentWorld->irs->metricPixelsPerPoint.x * camera->res->scale.x;
                params.wrapper.dst.h *= currentWorld->irs->metricPixelsPerPoint.y * camera->res->scale.y;

                params.wrapper.dst.x += ((gscope.activeResolution.width - params.wrapper.dst.w) / 2.0f - (params.camera_position.x - params.sourcePoint.x) * currentWorld->irs->metricPixelsPerPoint.x * camera->res->scale.x);
                params.wrapper.dst.y += ((gscope.activeResolution.height - params.wrapper.dst.h) / 2.0f + (params.camera_position.y - params.sourcePoint.y) * currentWorld->irs->metricPixelsPerPoint.y * camera->res->scale.y);

                SDL_RenderCopyExF(gscope.renderer, params.wrapper.texture, reinterpret_cast<SDL_Rect*>(&(params.wrapper.src)), reinterpret_cast<SDL_FRect*>(&(params.wrapper.dst)), lightSource->transform()->_angle_, nullptr, params.wrapper.flip);

                SDL_SetTextureColorMod(params.wrapper.texture, 255, 255, 255);
                SDL_SetTextureAlphaMod(params.wrapper.texture, 255);
                SDL_SetTextureBlendMode(params.wrapper.texture, SDL_BLENDMODE_BLEND);
            }
        }
        if(camera->visibleBorders)
        {
            float offset = 25 * std::max(1 - Time::deltaTime(), 0.1f);
            float height = 200 * Time::deltaTime();
            float w = static_cast<float>(gscope.activeResolution.width);
            float h = static_cast<float>(gscope.activeResolution.height);

            params.wrapper.dst.x = w / 2.0f;
            params.wrapper.dst.y = h / 2.0f;

            SDL_SetRenderDrawColor(gscope.renderer, 0, 255, 0, 25);

                   // Center dot
            SDL_RenderDrawPointF(gscope.renderer, params.wrapper.dst.x, params.wrapper.dst.y);
            SDL_RenderDrawPointF(gscope.renderer, params.wrapper.dst.x - offset, params.wrapper.dst.y);
            SDL_RenderDrawPointF(gscope.renderer, params.wrapper.dst.x + offset, params.wrapper.dst.y);
            SDL_RenderDrawPointF(gscope.renderer, params.wrapper.dst.x, params.wrapper.dst.y - offset);
            SDL_RenderDrawPointF(gscope.renderer, params.wrapper.dst.x, params.wrapper.dst.y + offset);

                   // borders (screen space)
            SDL_RenderDrawLineF(gscope.renderer, offset, offset, offset + height, offset);
            SDL_RenderDrawLineF(gscope.renderer, w - offset, offset, w - offset - height, offset);
            SDL_RenderDrawLineF(gscope.renderer, offset, h - offset, offset + height, h - offset);
            SDL_RenderDrawLineF(gscope.renderer, w - offset, h - offset, w - offset - height, h - offset);

            SDL_RenderDrawLineF(gscope.renderer, offset, 1 + offset, offset, offset + height);
            SDL_RenderDrawLineF(gscope.renderer, w - offset, 1 + offset, w - offset, offset + height);
            SDL_RenderDrawLineF(gscope.renderer, offset, h - 1 - offset, offset, h - offset - height);
            SDL_RenderDrawLineF(gscope.renderer, w - offset, h - 1 - offset, w - offset, h - offset - height);
        }

        if(camera->visibleObjects || camera->visibleNames)
        {
            Color prevColor = RenderUtility::GetColor();
            for(const auto &layer : params.orders)
            {
                for(Renderer *face : layer.second)
                {
                    if(!face || !face->transform())
                        continue;

                    Vec2 p = face->transform()->position() + face->get_offset();
                    if(camera->visibleObjects)
                    {
                        Rect factSz = face->get_relative_size();
                        Vec2 sz = Vec2::Scale(face->getSize(), Vec2(factSz.getWH()) / currentWorld->irs->metricPixelsPerPoint);
                        if(sz.x <= 0.f && sz.y <= 0.f)
                            sz = face->getSize();
                        RenderUtility::SetColor(Color::blue);
                        RenderUtility::DrawRectangleRotate(p, sz, face->transform()->angle() * Math::deg2rad);
                        RenderUtility::SetColor(Color::black);
                        RenderUtility::DrawPosition(p, 0.2f);
                    }
                    if(camera->visibleNames && face->gameObject())
                    {
                        RenderUtility::SetColor(Color::white);
                        RenderUtility::DrawText(p, face->gameObject()->m_name);
                    }
                }
            }
            RenderUtility::SetColor(prevColor);
        }

        for(auto &layer : params.orders)
        {
            layer.second.clear();
        }
    }

           /////////////////
           /// CAMERA 3D ///
           /////////////////
    void native_render_3D(Camera *camera)
    {
        if(camera == nullptr)
            return;
        if(camera->backclear)
        {
            RenderUtility::SetColor(camera->backcolor);
            SDL_RenderClear(gscope.renderer);
        }
    }

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
                            rendering->dst.w = static_cast<float>(target->sprite->m_rect.w);
                            rendering->dst.h = static_cast<float>(target->sprite->m_rect.h);
                            switch(target->renderPresentMode)
                            {
                                case SpriteRenderPresentMode::Fixed:
                                    break;
                                case SpriteRenderPresentMode::Place:
                                    rendering->src.w = std::min(rendering->src.w, static_cast<int>( target->sprite->width() * std::fabs(target->m_size.x)));
                                    rendering->src.h = std::min(rendering->src.h, static_cast<int>( target->sprite->height() * std::fabs(target->m_size.y)));
                                    break;
                            }
                            break;
                        case SpriteRenderType::Tile:
                        {
                            int tileW = std::max(1, target->sprite->width());
                            int tileH = std::max(1, target->sprite->height());
                            int totalW = std::max(1, static_cast<int>(std::fabs(target->m_size.x) * tileW));
                            int totalH = std::max(1, static_cast<int>(std::fabs(target->m_size.y) * tileH));

                            rendering->src.x = 0;
                            rendering->src.y = 0;
                            rendering->src.w = totalW;
                            rendering->src.h = totalH;
                            rendering->dst.w = static_cast<float>(tileW);
                            rendering->dst.h = static_cast<float>(tileH);

                            // generate tiles
                            target->save_texture = SDL_CreateTexture(
                                gscope.renderer,
                                defaultPixelFormat,
                                SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET,
                                totalW,
                                totalH);
                            if(target->save_texture == nullptr)
                            {
                                RoninSimulator::ShowMessageFail("Texture create fail");
                                break;
                            }

                            SDL_SetTextureBlendMode(target->save_texture, SDL_BLENDMODE_BLEND);
                            SDL_Texture *prevTarget = SDL_GetRenderTarget(gscope.renderer);
                            SDL_SetRenderTarget(gscope.renderer, target->save_texture);
                            SDL_SetRenderDrawColor(gscope.renderer, 0, 0, 0, 0);
                            SDL_RenderClear(gscope.renderer);

                            dest.w = tileW;
                            dest.h = tileH;

                            int tilesX = totalW / dest.w;
                            int tilesY = totalH / dest.h;

                            SDL_Texture *temp_texture = render_cache_texture(target->sprite.ptr_);
                            if(temp_texture != nullptr)
                            {
                                // render tile
                                switch(target->renderPresentMode)
                                {
                                    case SpriteRenderPresentMode::Fixed:
                                    {
                                        for(x = 0; x < tilesX; ++x)
                                        {
                                            for(y = 0; y < tilesY; ++y)
                                            {
                                                dest.x = x * dest.w;
                                                dest.y = y * dest.h;
                                                SDL_RenderCopy(gscope.renderer, temp_texture, reinterpret_cast<const SDL_Rect *>(&target->sprite->m_rect), &dest);
                                            }
                                        }
                                        break;
                                    }
                                    case SpriteRenderPresentMode::Place:
                                    {
                                        for(x = 0; x < tilesX; ++x)
                                        {
                                            for(y = 0; y < tilesY; ++y)
                                            {
                                                dest.x = x * dest.w;
                                                dest.y = y * dest.h;
                                                SDL_RenderCopy(gscope.renderer, temp_texture, reinterpret_cast<const SDL_Rect *>(&target->sprite->m_rect), &dest);
                                            }
                                        }
                                        // place remained
                                        int remW = totalW - tilesX * dest.w;
                                        int remH = totalH - tilesY * dest.h;
                                        if(remH > 0)
                                        {
                                            SDL_Rect srcPart = reinterpret_cast<const SDL_Rect &>(target->sprite->m_rect);
                                            srcPart.h = std::min(srcPart.h, remH);
                                            dest.y = tilesY * tileH;
                                            dest.h = remH;
                                            dest.w = tileW;
                                            for(x = 0; x < tilesX; ++x)
                                            {
                                                dest.x = x * tileW;
                                                SDL_RenderCopy(gscope.renderer, temp_texture, &srcPart, &dest);
                                            }
                                        }
                                        if(remW > 0)
                                        {
                                            SDL_Rect srcPart = reinterpret_cast<const SDL_Rect &>(target->sprite->m_rect);
                                            srcPart.w = std::min(srcPart.w, remW);
                                            dest.x = tilesX * tileW;
                                            dest.w = remW;
                                            dest.h = tileH;
                                            for(y = 0; y < tilesY; ++y)
                                            {
                                                dest.y = y * tileH;
                                                SDL_RenderCopy(gscope.renderer, temp_texture, &srcPart, &dest);
                                            }
                                            if(remH > 0)
                                            {
                                                srcPart.h = std::min(target->sprite->m_rect.h, remH);
                                                dest.y = tilesY * tileH;
                                                dest.h = remH;
                                                SDL_RenderCopy(gscope.renderer, temp_texture, &srcPart, &dest);
                                            }
                                        }
                                        break;
                                    }
                                }
                                render_cache_unref(temp_texture);
                            }
                            SDL_SetRenderTarget(gscope.renderer, prevTarget);
                            break;
                        }
                    }
                    target->save_src = rendering->src;
                    target->save_dst = rendering->dst;

                    if(target->save_texture)
                    {
                        SDL_GetTextureColorMod(target->save_texture, &target->prevColorTexture.r, &target->prevColorTexture.g, &target->prevColorTexture.b);
                        SDL_GetTextureAlphaMod(target->save_texture, &target->prevColorTexture.a);
                    }
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

                rendering->dst.w = rendering->dst.w * std::fabs(target->m_size.x) / currentWorld->irs->metricPixelsPerPoint.x;
                rendering->dst.h = rendering->dst.h * std::fabs(target->m_size.y) / currentWorld->irs->metricPixelsPerPoint.y;
                rendering->flip = static_cast<SDL_RendererFlip>(target->flip);

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
        if(target == nullptr || rendering == nullptr)
            return;
        switch(command)
        {
            case RenderCommand::PreRender:
            {
                Vec2 sz = target->getSize();
                rendering->dst.w = sz.x;
                rendering->dst.h = sz.y;
                rendering->texture = nullptr;
                break;
            }
            case RenderCommand::PostRender:
                break;
            default:;
        }
    }

} // namespace RoninEngine::Runtime
