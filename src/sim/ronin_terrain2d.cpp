#include "ronin.h"
#include "ronin_exception.h"
#include "NavMesh.h"
#include "ronin_matrix.h"
#include <algorithm>
#include <cmath>

using namespace RoninEngine::Runtime;
using namespace RoninEngine::AI;

struct RoninEngine::Runtime::T2Data
{
    RoninEngine::AI::NavMesh *nav2d = nullptr;
    TransformRef root = nullptr;
    Vec2 size = Vec2::zero;
    Vec2 textureScale = Vec2::one;
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
    render->renderPresentMode = SpriteRenderPresentMode::Place;
    render->renderType = SpriteRenderType::Simple;
    render->setSize(Vec2::Scale(state.getWH(), m_t2data->textureScale));

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
    if(sprite == nullptr)
        return;
    Vec2 origin(gridX * tileSize.x, gridY * tileSize.y);
    setMesh(Rectf(origin.x, origin.y, tileSize.x, tileSize.y), sprite, isCollider);
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
        if(textured.atlasIndex < 0 || textured.atlasIndex >= terrainData.atlasCount)
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
    if(!m_t2data || !m_t2data->root)
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

    if(m_t2data && m_t2data->nav2d)
    {
        rect.w = m_t2data->nav2d->getWidth();
        rect.h = m_t2data->nav2d->getHeight();
    }

    return rect;
}

void Terrain2D::free_render_cache()
{
    if(m_t2data && m_t2data->root)
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
