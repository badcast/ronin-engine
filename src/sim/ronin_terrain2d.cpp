#include "ronin.h"
#include "ronin_exception.h"
#include "NavMesh.h"
#include "ronin_matrix.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>

using namespace RoninEngine::Runtime;
using namespace RoninEngine::AI;

static std::uint64_t make_tile_key(int gridX, int gridY)
{
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(gridX)) << 32) |
           static_cast<std::uint64_t>(static_cast<std::uint32_t>(gridY));
}

static Color sample_surface_pixel(SDL_Surface *surf, int x, int y)
{
    if(!surf || !surf->pixels || !surf->format || surf->w <= 0 || surf->h <= 0)
        return Color::white;
    x = std::clamp(x, 0, surf->w - 1);
    y = std::clamp(y, 0, surf->h - 1);
    int bpp = surf->format->BytesPerPixel;
    const Uint8 *p = static_cast<const Uint8 *>(surf->pixels) + y * surf->pitch + x * bpp;
    Uint32 pixel = 0;
    switch(bpp)
    {
        case 1:
            pixel = *p;
            break;
        case 2:
            pixel = *reinterpret_cast<const Uint16 *>(p);
            break;
        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
                pixel = (p[0] << 16) | (p[1] << 8) | p[2];
            else
                pixel = p[0] | (p[1] << 8) | (p[2] << 16);
            break;
        case 4:
            pixel = *reinterpret_cast<const Uint32 *>(p);
            break;
        default:
            break;
    }
    Uint8 r = 255, g = 255, b = 255, a = 255;
    SDL_GetRGBA(pixel, surf->format, &r, &g, &b, &a);
    return Color(r, g, b, a);
}

struct RoninEngine::Runtime::T2Data
{
    RoninEngine::AI::NavMesh *nav2d = nullptr;
    TransformRef root = nullptr;
    Vec2 size = Vec2::zero;
    Vec2 textureScale = Vec2::one;
    std::unordered_map<std::uint64_t, SpriteRendererRef> tileGrid;
    Image *surface = nullptr;
    SDL_Texture *surfaceTexture = nullptr;
    bool surfaceDirty = false;
};

void Terrain2D::ensure_root()
{
    if(!m_t2data)
        return;
    if(m_t2data->root == nullptr)
    {
        m_t2data->root = Primitive::CreateEmptyGameObject("Grounds")->transform();
        if(transform())
        {
            m_t2data->root->setParent(transform(), false);
        }
    }
    else if(transform() && m_t2data->root->m_parent != transform())
    {
        m_t2data->root->setParent(transform(), false);
    }
}

void Terrain2D::update_navmesh_offset()
{
    if(!m_t2data || !m_t2data->nav2d)
        return;
    m_t2data->nav2d->worldScale = Vec2(0.1f, 0.1f);
    if(transform())
    {
        m_t2data->nav2d->worldOffset = -transform()->position();
    }
}

Terrain2D::Terrain2D() : Terrain2D(DESCRIBE_AS_MAIN_OFF(Terrain2D))
{
}

Terrain2D::Terrain2D(const std::string &name, float width, float height)
    : Renderer(name.empty() ? DESCRIBE_AS_ONLY_NAME(Terrain2D) : name)
{
    DESCRIBE_AS_MAIN(Terrain2D);
    _class = render_getclass<Terrain2D>();
    RoninMemory::alloc_self(m_t2data);
    m_t2data->size.x = std::max(1.0f, width);
    m_t2data->size.y = std::max(1.0f, height);
    RoninMemory::alloc_self(m_t2data->nav2d, static_cast<std::uint32_t>(m_t2data->size.x * 10), static_cast<std::uint32_t>(m_t2data->size.y * 10));
    m_t2data->nav2d->worldScale = Vec2(0.1f, 0.1f);
}

Terrain2D::Terrain2D(const Terrain2D &other) : Renderer(other.m_name)
{
    DESCRIBE_AS_MAIN(Terrain2D);
    _class = render_getclass<Terrain2D>();
    RoninMemory::alloc_self(m_t2data);
    if(other.m_t2data)
    {
        m_t2data->size = other.m_t2data->size;
        m_t2data->textureScale = other.m_t2data->textureScale;
        RoninMemory::alloc_self(m_t2data->nav2d, static_cast<std::uint32_t>(m_t2data->size.x * 10), static_cast<std::uint32_t>(m_t2data->size.y * 10));
        m_t2data->nav2d->worldScale = other.m_t2data->nav2d ? other.m_t2data->nav2d->worldScale : Vec2(0.1f, 0.1f);
        m_t2data->nav2d->worldOffset = other.m_t2data->nav2d ? other.m_t2data->nav2d->worldOffset : Vec2::zero;

        // Copy NavMesh locked obstacles
        if(other.m_t2data->nav2d)
        {
            std::uint32_t w = other.m_t2data->nav2d->getWidth();
            std::uint32_t h = other.m_t2data->nav2d->getHeight();
            for(std::uint32_t x = 0; x < w; ++x)
            {
                for(std::uint32_t y = 0; y < h; ++y)
                {
                    Vec2Int pt(x, y);
                    if(other.m_t2data->nav2d->hasLocked(pt))
                    {
                        m_t2data->nav2d->setLock(pt, true);
                    }
                }
            }
        }

        // Copy surface canvas if present
        if(other.m_t2data->surface)
        {
            m_t2data->surface = SDL_DuplicateSurface(other.m_t2data->surface);
            m_t2data->surfaceDirty = (m_t2data->surface != nullptr);
        }

        // Copy tiles
        if(other.m_t2data->root)
        {
            for(TransformRef child : other.m_t2data->root->hierarchy)
            {
                if(!child || !child->gameObject())
                    continue;
                SpriteRendererRef sr = child->gameObject()->GetComponent<SpriteRenderer>();
                if(sr && sr->getSprite())
                {
                    Vec2 localPos = child->localPosition();
                    Vec2 sz = sr->getSize();
                    setMesh(Rectf(localPos.x - sz.x / 2.0f, localPos.y - sz.y / 2.0f, sz.x, sz.y), sr->getSprite());
                }
            }
        }
    }
}

Terrain2D::~Terrain2D()
{
    if(m_t2data)
    {
        if(m_t2data->surfaceTexture)
        {
            SDL_DestroyTexture(m_t2data->surfaceTexture);
            m_t2data->surfaceTexture = nullptr;
        }
        if(m_t2data->surface)
        {
            SDL_FreeSurface(m_t2data->surface);
            m_t2data->surface = nullptr;
        }
        if(m_t2data->root && currentWorld && currentWorld->irs && !currentWorld->irs->requestUnloading)
        {
            if(!m_t2data->root->gameObject().isNull())
                m_t2data->root->gameObject()->Destroy();
        }
        if(m_t2data->nav2d)
            RoninMemory::free(m_t2data->nav2d);
        RoninMemory::free(m_t2data);
        m_t2data = nullptr;
    }
}

RoninEngine::AI::NavMesh *Terrain2D::get_navmesh2D()
{
    update_navmesh_offset();
    return m_t2data ? m_t2data->nav2d : nullptr;
}

void Terrain2D::setMesh(Rectf state, SpriteRef sprite)
{
    setMesh(state, sprite, false);
}

void Terrain2D::setMesh(Rectf state, SpriteRef sprite, bool isCollider)
{
    if(sprite == nullptr || !m_t2data)
        return;

    ensure_root();

    if(state.w <= 0.0f || state.h <= 0.0f)
    {
        Vec2 mpp = (currentWorld && currentWorld->irs) ? currentWorld->irs->metricPixelsPerPoint : Vec2(100.0f, 100.0f);
        if(state.w <= 0.0f)
            state.w = sprite->width() / mpp.x;
        if(state.h <= 0.0f)
            state.h = sprite->height() / mpp.y;
    }

    SpriteRendererRef render = Primitive::CreateEmptyGameObject()->AddComponent<SpriteRenderer>();
    TransformRef relative = render->transform();
    relative->setParent(m_t2data->root, false);
    render->setSprite(sprite);
    render->renderPresentMode = SpriteRenderPresentMode::Fixed;
    render->renderType = SpriteRenderType::Simple;
    Vec2 sprSize = sprite->size();
    Vec2 tileScale = Vec2::one;
    if(sprSize.x > 0.0f)
        tileScale.x = state.w / sprSize.x;
    if(sprSize.y > 0.0f)
        tileScale.y = state.h / sprSize.y;
    render->setSize(Vec2::Scale(tileScale, m_t2data->textureScale));

    relative->localPosition(state.getXY() + state.getWH() / 2.0f);

    if(_owner)
    {
        render->gameObject()->m_zOrder = _owner->m_zOrder;
        render->gameObject()->SetLayer(_owner->m_layer);
    }

    if(relative)
        Matrix::matrix_update(relative.ptr_);

    if(isCollider)
    {
        Vec2 origin = (transform() ? transform()->position() : Vec2::zero) + state.getXY();
        setCollider(Rectf(origin.x, origin.y, state.w, state.h), true);
    }
}

void Terrain2D::setMeshTile(int gridX, int gridY, SpriteRef sprite, Vec2 tileSize, bool isCollider)
{
    if(!m_t2data)
        return;
    if(sprite == nullptr)
    {
        removeMeshTile(gridX, gridY, tileSize);
        return;
    }

    Vec2 sz(std::max(0.01f, tileSize.x), std::max(0.01f, tileSize.y));
    Vec2 origin(gridX * sz.x, gridY * sz.y);
    Rectf state(origin.x, origin.y, sz.x, sz.y);

    std::uint64_t key = make_tile_key(gridX, gridY);
    auto it = m_t2data->tileGrid.find(key);
    if(it != m_t2data->tileGrid.end() && !it->second.isNull() && !it->second->gameObject().isNull())
    {
        SpriteRendererRef render = it->second;
        render->setSprite(sprite);
        Vec2 sprSize = sprite->size();
        Vec2 tileScale = Vec2::one;
        if(sprSize.x > 0.0f)
            tileScale.x = state.w / sprSize.x;
        if(sprSize.y > 0.0f)
            tileScale.y = state.h / sprSize.y;
        render->setSize(Vec2::Scale(tileScale, m_t2data->textureScale));
        if(render->transform())
        {
            render->transform()->localPosition(state.getXY() + state.getWH() / 2.0f);
            Matrix::matrix_update(render->transform().ptr_);
        }
        Vec2 worldOrigin = (transform() ? transform()->position() : Vec2::zero) + state.getXY();
        setCollider(Rectf(worldOrigin.x, worldOrigin.y, state.w, state.h), isCollider);
        return;
    }

    ensure_root();
    setMesh(state, sprite, isCollider);
    if(m_t2data->root && !m_t2data->root->hierarchy.empty())
    {
        TransformRef lastChild = m_t2data->root->hierarchy.back();
        if(lastChild && lastChild->gameObject())
        {
            SpriteRendererRef sr = lastChild->gameObject()->GetComponent<SpriteRenderer>();
            if(sr)
                m_t2data->tileGrid[key] = sr;
        }
    }
}

SpriteRendererRef Terrain2D::getMeshTile(int gridX, int gridY) const
{
    if(!m_t2data)
        return nullptr;
    auto it = m_t2data->tileGrid.find(make_tile_key(gridX, gridY));
    if(it != m_t2data->tileGrid.end() && !it->second.isNull() && !it->second->gameObject().isNull())
        return it->second;
    return nullptr;
}

bool Terrain2D::removeMeshTile(int gridX, int gridY, Vec2 tileSize)
{
    if(!m_t2data)
        return false;
    Vec2 sz(std::max(0.01f, tileSize.x), std::max(0.01f, tileSize.y));
    Vec2 worldOrigin = (transform() ? transform()->position() : Vec2::zero) + Vec2(gridX * sz.x, gridY * sz.y);
    setCollider(Rectf(worldOrigin.x, worldOrigin.y, sz.x, sz.y), false);

    std::uint64_t key = make_tile_key(gridX, gridY);
    auto it = m_t2data->tileGrid.find(key);
    if(it == m_t2data->tileGrid.end())
        return false;

    SpriteRendererRef sr = it->second;
    m_t2data->tileGrid.erase(it);
    if(!sr.isNull() && !sr->gameObject().isNull())
    {
        sr->gameObject()->Destroy();
        return true;
    }
    return false;
}

void Terrain2D::fillMeshTiles(int startX, int startY, int width, int height, SpriteRef sprite, Vec2 tileSize, bool isCollider)
{
    if(!m_t2data || width <= 0 || height <= 0)
        return;
    for(int y = startY; y < startY + height; ++y)
    {
        for(int x = startX; x < startX + width; ++x)
        {
            setMeshTile(x, y, sprite, tileSize, isCollider);
        }
    }
}

void Terrain2D::setTextureScale(Vec2 scale)
{
    if(m_t2data)
        m_t2data->textureScale = scale;
}

Vec2 Terrain2D::getTextureScale() const
{
    return m_t2data ? m_t2data->textureScale : Vec2::one;
}

void Terrain2D::createSurface(int pixelWidth, int pixelHeight, Color clearColor)
{
    if(!m_t2data)
        return;
    if(pixelWidth <= 0)
        pixelWidth = std::clamp(static_cast<int>(std::round(m_t2data->size.x * 32.0f)), 16, 2048);
    if(pixelHeight <= 0)
        pixelHeight = std::clamp(static_cast<int>(std::round(m_t2data->size.y * 32.0f)), 16, 2048);

    if(m_t2data->surfaceTexture)
    {
        SDL_DestroyTexture(m_t2data->surfaceTexture);
        m_t2data->surfaceTexture = nullptr;
    }
    if(m_t2data->surface && (m_t2data->surface->w != pixelWidth || m_t2data->surface->h != pixelHeight))
    {
        SDL_FreeSurface(m_t2data->surface);
        m_t2data->surface = nullptr;
    }
    if(!m_t2data->surface)
    {
        m_t2data->surface = SDL_CreateRGBSurfaceWithFormat(0, pixelWidth, pixelHeight, 32, defaultPixelFormat);
        if(m_t2data->surface)
            SDL_SetSurfaceBlendMode(m_t2data->surface, SDL_BLENDMODE_BLEND);
    }
    clearSurface(clearColor);
}

Image *Terrain2D::getSurface() const
{
    return m_t2data ? m_t2data->surface : nullptr;
}

SpriteRef Terrain2D::getSurfaceSprite()
{
    if(!m_t2data)
        return nullptr;
    if(!m_t2data->surface)
        createSurface();
    if(!m_t2data->surface)
        return nullptr;
    return Primitive::CreateSpriteFrom(m_t2data->surface);
}

bool Terrain2D::hasSurface() const
{
    return m_t2data && m_t2data->surface != nullptr;
}

void Terrain2D::clearSurface(Color clearColor)
{
    if(!m_t2data || !m_t2data->surface)
        return;
    Uint32 mapped = SDL_MapRGBA(m_t2data->surface->format, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    SDL_FillRect(m_t2data->surface, nullptr, mapped);
    m_t2data->surfaceDirty = true;
}

void Terrain2D::fillSurface(Color color)
{
    if(!m_t2data)
        return;
    if(!m_t2data->surface)
        createSurface(0, 0, color);
    else
        clearSurface(color);
}

void Terrain2D::paintPixel(int px, int py, Color color, bool blend)
{
    if(!m_t2data)
        return;
    if(!m_t2data->surface)
        createSurface();
    SDL_Surface *surf = m_t2data->surface;
    if(!surf || !surf->pixels || px < 0 || py < 0 || px >= surf->w || py >= surf->h)
        return;

    Uint8 *p = static_cast<Uint8 *>(surf->pixels) + py * surf->pitch + px * 4;
    Uint32 *pixelPtr = reinterpret_cast<Uint32 *>(p);

    if(!blend || color.a == 255)
    {
        *pixelPtr = SDL_MapRGBA(surf->format, color.r, color.g, color.b, color.a);
    }
    else if(color.a > 0)
    {
        Uint8 dstR = 0, dstG = 0, dstB = 0, dstA = 0;
        SDL_GetRGBA(*pixelPtr, surf->format, &dstR, &dstG, &dstB, &dstA);
        float sa = color.a / 255.0f;
        float da = (dstA / 255.0f) * (1.0f - sa);
        float outA = sa + da;
        if(outA > 1e-5f)
        {
            Uint8 r = static_cast<Uint8>(std::clamp((color.r * sa + dstR * da) / outA, 0.0f, 255.0f));
            Uint8 g = static_cast<Uint8>(std::clamp((color.g * sa + dstG * da) / outA, 0.0f, 255.0f));
            Uint8 b = static_cast<Uint8>(std::clamp((color.b * sa + dstB * da) / outA, 0.0f, 255.0f));
            Uint8 a = static_cast<Uint8>(std::clamp(outA * 255.0f, 0.0f, 255.0f));
            *pixelPtr = SDL_MapRGBA(surf->format, r, g, b, a);
        }
    }
    m_t2data->surfaceDirty = true;
}

Color Terrain2D::getPixel(int px, int py) const
{
    if(!m_t2data || !m_t2data->surface)
        return Color(0, 0, 0, 0);
    if(px < 0 || py < 0 || px >= m_t2data->surface->w || py >= m_t2data->surface->h)
        return Color(0, 0, 0, 0);
    return sample_surface_pixel(m_t2data->surface, px, py);
}

void Terrain2D::paintRect(Rectf localRect, Color color, bool blend)
{
    if(!m_t2data)
        return;
    if(!m_t2data->surface)
        createSurface();
    SDL_Surface *surf = m_t2data->surface;
    if(!surf || m_t2data->size.x <= 0.0f || m_t2data->size.y <= 0.0f)
        return;

    float scaleX = surf->w / m_t2data->size.x;
    float scaleY = surf->h / m_t2data->size.y;
    int x0 = std::clamp(static_cast<int>(std::floor(localRect.x * scaleX)), 0, surf->w);
    int y0 = std::clamp(static_cast<int>(std::floor(localRect.y * scaleY)), 0, surf->h);
    int x1 = std::clamp(static_cast<int>(std::ceil((localRect.x + localRect.w) * scaleX)), 0, surf->w);
    int y1 = std::clamp(static_cast<int>(std::ceil((localRect.y + localRect.h) * scaleY)), 0, surf->h);

    for(int y = y0; y < y1; ++y)
    {
        for(int x = x0; x < x1; ++x)
        {
            paintPixel(x, y, color, blend);
        }
    }
}

void Terrain2D::paintCircle(Vec2 localPos, float radiusWorld, Color color, bool blend)
{
    paintBrush(localPos, radiusWorld, color, 1.0f);
    (void)blend;
}

void Terrain2D::paintBrush(Vec2 localPos, float radiusWorld, Color color, float hardness)
{
    if(!m_t2data || radiusWorld <= 0.0f)
        return;
    if(!m_t2data->surface)
        createSurface();
    SDL_Surface *surf = m_t2data->surface;
    if(!surf || m_t2data->size.x <= 0.0f || m_t2data->size.y <= 0.0f)
        return;

    hardness = std::clamp(hardness, 0.0f, 1.0f);
    float scaleX = surf->w / m_t2data->size.x;
    float scaleY = surf->h / m_t2data->size.y;
    float cx = localPos.x * scaleX;
    float cy = localPos.y * scaleY;
    float rx = std::max(1.0f, radiusWorld * scaleX);
    float ry = std::max(1.0f, radiusWorld * scaleY);

    int minX = std::clamp(static_cast<int>(std::floor(cx - rx)), 0, surf->w - 1);
    int maxX = std::clamp(static_cast<int>(std::ceil(cx + rx)), 0, surf->w - 1);
    int minY = std::clamp(static_cast<int>(std::floor(cy - ry)), 0, surf->h - 1);
    int maxY = std::clamp(static_cast<int>(std::ceil(cy + ry)), 0, surf->h - 1);

    for(int y = minY; y <= maxY; ++y)
    {
        for(int x = minX; x <= maxX; ++x)
        {
            float nx = (x + 0.5f - cx) / rx;
            float ny = (y + 0.5f - cy) / ry;
            float dist = std::sqrt(nx * nx + ny * ny);
            if(dist <= 1.0f)
            {
                float falloff = 1.0f;
                if(hardness < 0.999f && dist > hardness)
                {
                    falloff = 1.0f - (dist - hardness) / std::max(0.001f, 1.0f - hardness);
                    falloff = std::clamp(falloff, 0.0f, 1.0f);
                }
                Color c = color;
                c.a = static_cast<uint8_t>(std::round(color.a * falloff));
                paintPixel(x, y, c, true);
            }
        }
    }
}

void Terrain2D::paintLine(Vec2 localFrom, Vec2 localTo, float radiusWorld, Color color, float hardness)
{
    float dist = Vec2::Distance(localFrom, localTo);
    float step = std::max(0.02f, radiusWorld * 0.25f);
    int steps = std::max(1, static_cast<int>(std::ceil(dist / step)));
    for(int i = 0; i <= steps; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        Vec2 p = Vec2::Lerp(localFrom, localTo, t);
        paintBrush(p, radiusWorld, color, hardness);
    }
}

void Terrain2D::paintTexture(Rectf localRect, SpriteRef textureSprite, Color tint, bool tileRepeat)
{
    if(!m_t2data || textureSprite.isNull() || !textureSprite->surface)
        return;
    if(!m_t2data->surface)
        createSurface();
    SDL_Surface *surf = m_t2data->surface;
    SDL_Surface *srcSurf = textureSprite->surface;
    Rect srcRect = textureSprite->rect();
    if(srcRect.w <= 0 || srcRect.h <= 0)
    {
        srcRect = Rect(0, 0, srcSurf->w, srcSurf->h);
    }
    if(srcRect.w <= 0 || srcRect.h <= 0)
        return;

    float scaleX = surf->w / m_t2data->size.x;
    float scaleY = surf->h / m_t2data->size.y;
    int x0 = std::clamp(static_cast<int>(std::floor(localRect.x * scaleX)), 0, surf->w);
    int y0 = std::clamp(static_cast<int>(std::floor(localRect.y * scaleY)), 0, surf->h);
    int x1 = std::clamp(static_cast<int>(std::ceil((localRect.x + localRect.w) * scaleX)), 0, surf->w);
    int y1 = std::clamp(static_cast<int>(std::ceil((localRect.y + localRect.h) * scaleY)), 0, surf->h);
    int rw = std::max(1, x1 - x0);
    int rh = std::max(1, y1 - y0);

    for(int y = y0; y < y1; ++y)
    {
        for(int x = x0; x < x1; ++x)
        {
            int sx = 0, sy = 0;
            if(tileRepeat)
            {
                sx = srcRect.x + ((x - x0) % srcRect.w);
                sy = srcRect.y + ((y - y0) % srcRect.h);
            }
            else
            {
                sx = srcRect.x + ((x - x0) * srcRect.w) / rw;
                sy = srcRect.y + ((y - y0) * srcRect.h) / rh;
            }
            Color texCol = sample_surface_pixel(srcSurf, sx, sy);
            Color finalCol(
                static_cast<int>((texCol.r * tint.r) / 255),
                static_cast<int>((texCol.g * tint.g) / 255),
                static_cast<int>((texCol.b * tint.b) / 255),
                static_cast<int>((texCol.a * tint.a) / 255));
            paintPixel(x, y, finalCol, true);
        }
    }
}

void Terrain2D::paintBrushTexture(Vec2 localPos, float radiusWorld, SpriteRef textureSprite, Color tint, float hardness)
{
    if(!m_t2data || radiusWorld <= 0.0f || textureSprite.isNull() || !textureSprite->surface)
        return;
    if(!m_t2data->surface)
        createSurface();
    SDL_Surface *surf = m_t2data->surface;
    SDL_Surface *srcSurf = textureSprite->surface;
    Rect srcRect = textureSprite->rect();
    if(srcRect.w <= 0 || srcRect.h <= 0)
        srcRect = Rect(0, 0, srcSurf->w, srcSurf->h);
    if(srcRect.w <= 0 || srcRect.h <= 0)
        return;

    hardness = std::clamp(hardness, 0.0f, 1.0f);
    float scaleX = surf->w / m_t2data->size.x;
    float scaleY = surf->h / m_t2data->size.y;
    float cx = localPos.x * scaleX;
    float cy = localPos.y * scaleY;
    float rx = std::max(1.0f, radiusWorld * scaleX);
    float ry = std::max(1.0f, radiusWorld * scaleY);

    int minX = std::clamp(static_cast<int>(std::floor(cx - rx)), 0, surf->w - 1);
    int maxX = std::clamp(static_cast<int>(std::ceil(cx + rx)), 0, surf->w - 1);
    int minY = std::clamp(static_cast<int>(std::floor(cy - ry)), 0, surf->h - 1);
    int maxY = std::clamp(static_cast<int>(std::ceil(cy + ry)), 0, surf->h - 1);

    for(int y = minY; y <= maxY; ++y)
    {
        for(int x = minX; x <= maxX; ++x)
        {
            float nx = (x + 0.5f - cx) / rx;
            float ny = (y + 0.5f - cy) / ry;
            float dist = std::sqrt(nx * nx + ny * ny);
            if(dist <= 1.0f)
            {
                float falloff = 1.0f;
                if(hardness < 0.999f && dist > hardness)
                {
                    falloff = 1.0f - (dist - hardness) / std::max(0.001f, 1.0f - hardness);
                    falloff = std::clamp(falloff, 0.0f, 1.0f);
                }
                int sx = srcRect.x + (x % srcRect.w);
                int sy = srcRect.y + (y % srcRect.h);
                Color texCol = sample_surface_pixel(srcSurf, sx, sy);
                Color finalCol(
                    static_cast<int>((texCol.r * tint.r) / 255),
                    static_cast<int>((texCol.g * tint.g) / 255),
                    static_cast<int>((texCol.b * tint.b) / 255),
                    static_cast<int>(((texCol.a * tint.a) / 255) * falloff));
                paintPixel(x, y, finalCol, true);
            }
        }
    }
}

void Terrain2D::eraseBrush(Vec2 localPos, float radiusWorld, float hardness)
{
    if(!m_t2data || !m_t2data->surface || radiusWorld <= 0.0f)
        return;
    SDL_Surface *surf = m_t2data->surface;
    hardness = std::clamp(hardness, 0.0f, 1.0f);
    float scaleX = surf->w / m_t2data->size.x;
    float scaleY = surf->h / m_t2data->size.y;
    float cx = localPos.x * scaleX;
    float cy = localPos.y * scaleY;
    float rx = std::max(1.0f, radiusWorld * scaleX);
    float ry = std::max(1.0f, radiusWorld * scaleY);

    int minX = std::clamp(static_cast<int>(std::floor(cx - rx)), 0, surf->w - 1);
    int maxX = std::clamp(static_cast<int>(std::ceil(cx + rx)), 0, surf->w - 1);
    int minY = std::clamp(static_cast<int>(std::floor(cy - ry)), 0, surf->h - 1);
    int maxY = std::clamp(static_cast<int>(std::ceil(cy + ry)), 0, surf->h - 1);

    for(int y = minY; y <= maxY; ++y)
    {
        for(int x = minX; x <= maxX; ++x)
        {
            float nx = (x + 0.5f - cx) / rx;
            float ny = (y + 0.5f - cy) / ry;
            float dist = std::sqrt(nx * nx + ny * ny);
            if(dist <= 1.0f)
            {
                float eraseFactor = 1.0f;
                if(hardness < 0.999f && dist > hardness)
                {
                    eraseFactor = 1.0f - (dist - hardness) / std::max(0.001f, 1.0f - hardness);
                    eraseFactor = std::clamp(eraseFactor, 0.0f, 1.0f);
                }
                Uint8 *p = static_cast<Uint8 *>(surf->pixels) + y * surf->pitch + x * 4;
                Uint32 *pixelPtr = reinterpret_cast<Uint32 *>(p);
                Uint8 r = 0, g = 0, b = 0, a = 0;
                SDL_GetRGBA(*pixelPtr, surf->format, &r, &g, &b, &a);
                Uint8 newA = static_cast<Uint8>(std::round(a * (1.0f - eraseFactor)));
                *pixelPtr = SDL_MapRGBA(surf->format, r, g, b, newA);
            }
        }
    }
    m_t2data->surfaceDirty = true;
}

native_texture_t *Terrain2D::get_render_texture()
{
    if(!m_t2data || !m_t2data->surface || gscope.renderer == nullptr)
        return nullptr;

    if(m_t2data->surfaceTexture == nullptr)
    {
        m_t2data->surfaceTexture = SDL_CreateTextureFromSurface(gscope.renderer, m_t2data->surface);
        if(m_t2data->surfaceTexture)
        {
            SDL_SetTextureBlendMode(m_t2data->surfaceTexture, SDL_BLENDMODE_BLEND);
        }
        m_t2data->surfaceDirty = false;
    }
    else if(m_t2data->surfaceDirty)
    {
        SDL_UpdateTexture(m_t2data->surfaceTexture, nullptr, m_t2data->surface->pixels, m_t2data->surface->pitch);
        m_t2data->surfaceDirty = false;
    }

    return m_t2data->surfaceTexture;
}

void Terrain2D::loadFrom(const TerrainData &terrainData, bool clearExisting)
{
    if(!m_t2data || !terrainData.textured || !terrainData.atlases)
        return;

    if(clearExisting)
    {
        clearMesh();
    }

    for(int i = 0; i < terrainData.dataCount; ++i)
    {
        const TerrainTextured &textured = terrainData.textured[i];
        if(textured.atlasIndex < 0 || textured.atlasIndex >= terrainData.atlasCount || !textured.texturedRegion)
            continue;
        Atlas &atlas = terrainData.atlases[textured.atlasIndex];
        for(std::uint32_t r = 0; r < textured.regionCount; ++r)
        {
            const TerrainRegion &reg = textured.texturedRegion[r];
            SpriteRef spr = atlas.GetSpriteFromIndex(reg.textureIndex);
            if(spr)
            {
                setMesh(Rectf(reg.region.x, reg.region.y, reg.region.w, reg.region.h), spr);
            }
        }
    }
}

void Terrain2D::setCollider(const Rectf &worldRect, bool state)
{
    if(!m_t2data || !m_t2data->nav2d)
        return;

    update_navmesh_offset();

    Vec2Int pMin = m_t2data->nav2d->WorldPointToPoint(worldRect.getXY());
    Vec2Int pMax = m_t2data->nav2d->WorldPointToPoint(worldRect.getXY() + worldRect.getWH());

    int rMinX = std::min(pMin.x, pMax.x);
    int rMaxX = std::max(pMin.x, pMax.x);
    int rMinY = std::min(pMin.y, pMax.y);
    int rMaxY = std::max(pMin.y, pMax.y);

    int w = static_cast<int>(m_t2data->nav2d->getWidth());
    int h = static_cast<int>(m_t2data->nav2d->getHeight());

    if(rMaxX < 0 || rMinX >= w || rMaxY < 0 || rMinY >= h)
        return;

    int minX = std::clamp(rMinX, 0, w - 1);
    int maxX = std::clamp(rMaxX, 0, w - 1);
    int minY = std::clamp(rMinY, 0, h - 1);
    int maxY = std::clamp(rMaxY, 0, h - 1);

    for(int x = minX; x <= maxX; ++x)
    {
        for(int y = minY; y <= maxY; ++y)
        {
            m_t2data->nav2d->setLock(Vec2Int(x, y), state);
        }
    }
}

void Terrain2D::setCollider(const Vec2 &worldPoint, bool state)
{
    if(!m_t2data || !m_t2data->nav2d)
        return;
    update_navmesh_offset();
    Vec2Int pt = m_t2data->nav2d->WorldPointToPoint(worldPoint);
    if(m_t2data->nav2d->Contains(pt))
    {
        m_t2data->nav2d->setLock(pt, state);
    }
}

void Terrain2D::setColliderCircle(const Vec2 &worldCenter, float radius, bool state)
{
    if(!m_t2data || !m_t2data->nav2d || radius <= 0.0f)
        return;
    update_navmesh_offset();
    Rectf bound(worldCenter.x - radius, worldCenter.y - radius, radius * 2.0f, radius * 2.0f);
    Vec2Int pMin = m_t2data->nav2d->WorldPointToPoint(bound.getXY());
    Vec2Int pMax = m_t2data->nav2d->WorldPointToPoint(bound.getXY() + bound.getWH());

    int rMinX = std::min(pMin.x, pMax.x);
    int rMaxX = std::max(pMin.x, pMax.x);
    int rMinY = std::min(pMin.y, pMax.y);
    int rMaxY = std::max(pMin.y, pMax.y);

    int w = static_cast<int>(m_t2data->nav2d->getWidth());
    int h = static_cast<int>(m_t2data->nav2d->getHeight());

    if(rMaxX < 0 || rMinX >= w || rMaxY < 0 || rMinY >= h)
        return;

    int minX = std::clamp(rMinX, 0, w - 1);
    int maxX = std::clamp(rMaxX, 0, w - 1);
    int minY = std::clamp(rMinY, 0, h - 1);
    int maxY = std::clamp(rMaxY, 0, h - 1);

    float rSq = radius * radius;
    for(int x = minX; x <= maxX; ++x)
    {
        for(int y = minY; y <= maxY; ++y)
        {
            Vec2 worldPt = m_t2data->nav2d->PointToWorldPoint(x, y);
            float dx = worldCenter.x - worldPt.x;
            float dy = worldCenter.y - worldPt.y;
            if((dx * dx + dy * dy) <= rSq)
            {
                m_t2data->nav2d->setLock(Vec2Int(x, y), state);
            }
        }
    }
}

void Terrain2D::clearColliders()
{
    if(m_t2data && m_t2data->nav2d)
    {
        m_t2data->nav2d->Clear(true);
    }
}

void Terrain2D::clearMesh()
{
    if(!m_t2data)
        return;
    m_t2data->tileGrid.clear();
    if(!m_t2data->root)
        return;
    std::vector<TransformRef> children = m_t2data->root->hierarchy;
    for(TransformRef child : children)
    {
        if(child && child->gameObject())
        {
            child->gameObject()->Destroy();
        }
    }
}

void Terrain2D::clear()
{
    clearMesh();
    clearColliders();
    clearSurface();
}

TransformRef Terrain2D::getRoot() const
{
    return m_t2data ? m_t2data->root : nullptr;
}

std::vector<SpriteRendererRef> Terrain2D::getTiles() const
{
    std::vector<SpriteRendererRef> result;
    if(m_t2data && m_t2data->root)
    {
        for(TransformRef child : m_t2data->root->hierarchy)
        {
            if(child && child->gameObject())
            {
                SpriteRendererRef sr = child->gameObject()->GetComponent<SpriteRenderer>();
                if(sr)
                    result.push_back(sr);
            }
        }
    }
    return result;
}

std::size_t Terrain2D::getTileCount() const
{
    return (m_t2data && m_t2data->root) ? m_t2data->root->hierarchy.size() : 0;
}

void Terrain2D::setZOrder(int zOrder)
{
    if(_owner)
        _owner->m_zOrder = zOrder;
    if(m_t2data && m_t2data->root)
    {
        for(TransformRef child : m_t2data->root->hierarchy)
        {
            if(child && child->gameObject())
                child->gameObject()->m_zOrder = zOrder;
        }
    }
}

void Terrain2D::setLayer(int layer)
{
    if(_owner)
        _owner->SetLayer(layer);
    if(m_t2data && m_t2data->root)
    {
        for(TransformRef child : m_t2data->root->hierarchy)
        {
            if(child && child->gameObject())
                child->gameObject()->SetLayer(layer);
        }
    }
}

const bool Terrain2D::is_collider(const Vec2 destination)
{
    if(!m_t2data || !m_t2data->nav2d)
        return false;
    update_navmesh_offset();
    auto n = m_t2data->nav2d->get(destination);
    return n && m_t2data->nav2d->hasLocked(n);
}

const Vec2 Terrain2D::getSize() const
{
    return m_t2data ? m_t2data->size : Vec2::zero;
}

void Terrain2D::setSize(const Vec2 &value)
{
    if(!m_t2data)
        return;
    Vec2 newSize = Vec2::Max(Vec2::one, Vec2::Abs(value));
    if(m_t2data->nav2d)
        RoninMemory::free(m_t2data->nav2d);
    RoninMemory::alloc_self(m_t2data->nav2d, static_cast<std::uint32_t>(newSize.x * 10), static_cast<std::uint32_t>(newSize.y * 10));
    m_t2data->nav2d->worldScale = Vec2(0.1f, 0.1f);
    if(transform())
        m_t2data->nav2d->worldOffset = -transform()->position();
    m_t2data->size = newSize;

    if(_owner && transform())
        Matrix::matrix_update(transform().ptr_);
}

Vec2 Terrain2D::get_offset()
{
    return {};
}

Rect Terrain2D::get_relative_size()
{
    Rect rect;

    if(m_t2data && m_t2data->surface)
    {
        rect.w = m_t2data->surface->w;
        rect.h = m_t2data->surface->h;
    }
    else if(m_t2data && m_t2data->nav2d)
    {
        rect.w = m_t2data->nav2d->getWidth();
        rect.h = m_t2data->nav2d->getHeight();
    }

    return rect;
}

void Terrain2D::free_render_cache()
{
    if(m_t2data)
    {
        if(m_t2data->surfaceTexture)
        {
            SDL_DestroyTexture(m_t2data->surfaceTexture);
            m_t2data->surfaceTexture = nullptr;
            m_t2data->surfaceDirty = true;
        }
        if(m_t2data->root)
        {
            for(TransformRef child : m_t2data->root->hierarchy)
            {
                if(!child.isNull() && !child->gameObject().isNull())
                {
                    SpriteRendererRef sr = child->gameObject()->GetComponent<SpriteRenderer>();
                    if(!sr.isNull())
                        sr->free_render_cache();
                }
            }
        }
    }
}
