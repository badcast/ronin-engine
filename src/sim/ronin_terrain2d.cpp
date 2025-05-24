
#include "ronin.h"
#include "ronin_exception.h"
#include "NavMesh.h"

using namespace RoninEngine::Runtime;
using namespace RoninEngine::AI;

struct RoninEngine::Runtime::T2Data
{
    RoninEngine::AI::NavMesh *nav2d;
    TransformRef root;
    Vec2 size;
};

Terrain2D::Terrain2D() : Terrain2D(DESCRIBE_AS_MAIN_OFF(Terrain2D))
{
}

Terrain2D::Terrain2D(const std::string &name, float width, float height) : Renderer(DESCRIBE_AS_ONLY_NAME(Terrain2D))
{
    DESCRIBE_AS_MAIN(Terrain2D);
    _class = render_getclass<Terrain2D>();
    RoninMemory::alloc_self(m_t2data);
    m_t2data->size.x = width;
    m_t2data->size.y = height;
    RoninMemory::alloc_self(m_t2data->nav2d, static_cast<std::uint32_t>(m_t2data->size.x * 10), static_cast<std::uint32_t>(m_t2data->size.y * 10));
}

Terrain2D::~Terrain2D()
{
    RoninMemory::free(m_t2data->nav2d);
    RoninMemory::free(m_t2data);
}

NavMesh *Terrain2D::get_navmesh2D()
{
    return m_t2data->nav2d;
}

void Terrain2D::setMesh(Rectf state, SpriteRef sprite)
{
    SpriteRendererRef render;
    TransformRef relative;
    if(sprite == nullptr)
        return;
    if(m_t2data->root == nullptr)
    {
        m_t2data->root = Primitive::CreateEmptyGameObject("Grounds")->transform();
        m_t2data->root->setParent(transform(), false);
    }
    render = Primitive::CreateEmptyGameObject()->AddComponent<SpriteRenderer>();
    relative = render->transform();
    render->transform()->setParent(m_t2data->root);
    render->setSprite(sprite);
    render->renderPresentMode = SpriteRenderPresentMode::Place;
    render->renderType = SpriteRenderType::Simple;

    Vec2 firstPoint = relative->position();

    relative->position(firstPoint + state.getXY());
}

void Terrain2D::setTextureScale(Vec2 scale)
{
}

void Terrain2D::loadFrom(const TerrainData &terrainData)
{
    throw Exception::ronin_implementation_error();
}

const bool Terrain2D::is_collider(const Vec2 destination)
{
    auto n = m_t2data->nav2d->get(destination);
    return n && m_t2data->nav2d->hasLocked(n);
}

const Vec2 Terrain2D::getSize() const
{
    return m_t2data->size;
}

void Terrain2D::setSize(const Vec2 &value)
{
    if(m_t2data->nav2d)
        RoninMemory::free(m_t2data->nav2d);
    RoninMemory::alloc_self(m_t2data->nav2d, static_cast<std::uint32_t>(value.x * 10), static_cast<std::uint32_t>(value.y * 10));
    m_t2data->size = value;
}

Vec2 Terrain2D::get_offset()
{
    return {};
}

Rect Terrain2D::get_relative_size()
{
    Rect rect;

    if(m_t2data->nav2d)
    {
        rect.w = m_t2data->nav2d->getWidth();
        rect.h = m_t2data->nav2d->getHeight();
    }

    return rect;
}

void Terrain2D::free_render_cache()
{
    // TODO: Free render cache for Terrain2D
}
