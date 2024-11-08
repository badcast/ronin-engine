#include "ronin.h"
#include "ronin_render_cache.h"

namespace RoninEngine::Runtime
{
    // FIXME: OPTIMIZING SPRITE RENDERER COMPONENT

    SpriteRenderer::SpriteRenderer() : SpriteRenderer(DESCRIBE_AS_MAIN_OFF(SpriteRenderer))
    {
    }

    SpriteRenderer::SpriteRenderer(const std::string &name)
        : Renderer(DESCRIBE_AS_ONLY_NAME(SpriteRenderer)), sprite(nullptr), save_texture(nullptr), renderType(SpriteRenderType::Simple), renderOut(SpriteRenderOut::Centering), flip(SpriteRenderFlip::FlipNone), renderPresentMode(SpriteRenderPresentMode::Fixed), color(Color::white)
    {
        _class = render_getclass<SpriteRenderer>();
        DESCRIBE_AS_MAIN(SpriteRenderer);
    }

    SpriteRenderer::SpriteRenderer(const SpriteRenderer &proto) : Renderer(proto.m_name), sprite(proto.sprite), save_texture(nullptr), m_size(proto.m_size), renderType(proto.renderType), renderOut(proto.renderOut), flip(proto.flip), renderPresentMode(proto.renderPresentMode), color(proto.color)
    {
        _class = render_getclass<SpriteRenderer>();
    }

    SpriteRenderer::~SpriteRenderer()
    {
        free_render_cache();
    }

    const SpriteRenderType SpriteRenderer::getRenderType() const
    {
        return renderType;
    }

    void SpriteRenderer::setRenderType(SpriteRenderType value)
    {
        free_render_cache();
        renderType = value;
    }

    const SpriteRenderOut SpriteRenderer::getRenderOut() const
    {
        return renderOut;
    }

    void SpriteRenderer::setRenderOut(SpriteRenderOut value)
    {
        free_render_cache();
        renderOut = value;
    }

    const SpriteRenderPresentMode SpriteRenderer::getPresentMode() const
    {
        return renderPresentMode;
    }

    void SpriteRenderer::setPresentMode(SpriteRenderPresentMode value)
    {
        free_render_cache();
        renderPresentMode = value;
    }

    const Color SpriteRenderer::getColor() const
    {
        return color;
    }

    void SpriteRenderer::setColor(Color value)
    {
        color = value;
    }

    const Vec2 SpriteRenderer::getSize() const
    {
        return this->m_size;
    }

    void SpriteRenderer::setSize(const Vec2 &newSize)
    {
        m_size = Vec2::Abs(newSize);
    }

    void SpriteRenderer::setRealSize()
    {
        if(sprite)
            setSize(sprite->size());
    }

    Vec2 SpriteRenderer::get_offset()
    {
        Vec2 outOffset;
        switch(renderOut)
        {
            case SpriteRenderOut::Origin:
                outOffset = -m_size + m_size / 2 + Vec2::half;

                break;
        }
        return outOffset;
    }

    Rect SpriteRenderer::get_relative_size()
    {
        Rect rect;
        if(sprite)
        {
            rect = sprite->realityRect();
        }
        return rect;
    }

    void SpriteRenderer::setSprite(SpriteRef sprite)
    {
        if(this->sprite.isNull() && (!this->m_size.x || !this->m_size.y))
        {
            this->m_size = Vec2::one;
        }
        this->sprite = sprite;

        // cache free from last sprite
        free_render_cache();
    }

    const SpriteRef SpriteRenderer::getSprite() const
    {
        return this->sprite;
    }

    void SpriteRenderer::free_render_cache()
    {
        if(save_texture)
        {
            render_cache_unref(save_texture);
            save_texture = nullptr;
        }
    }

} // namespace RoninEngine::Runtime
