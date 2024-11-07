
#include "ronin.h"
#include "ronin_exception.h"
#include "NavMesh.h"

using namespace RoninEngine::Runtime;
using namespace RoninEngine::AI;

struct RoninEngine::Runtime::T2Data
{
    RoninEngine::AI::NavMesh *nav2d;
    Transform* root;
    Vec2 size;
};

Terrain2D::Terrain2D() : Terrain2D(DESCRIBE_AS_MAIN_OFF(Terrain2D))
{}

Terrain2D::Terrain2D(const std::string &name, float width, float height) : Renderer(DESCRIBE_AS_ONLY_NAME(Terrain2D))
{
    DESCRIBE_AS_MAIN(Terrain2D);
    _class = render_getclass<Terrain2D>();
    RoninMemory::alloc_self(ref);
    ref->size.x = width;
    ref->size.y = height;
    RoninMemory::alloc_self(ref->nav2d, static_cast<std::uint32_t>(ref->size.x * 10), static_cast<std::uint32_t>(ref->size.y * 10));
}

Terrain2D::~Terrain2D()
{
    RoninMemory::free(ref->nav2d);
    RoninMemory::free(ref);
}

NavMesh *Terrain2D::get_navmesh2D()
{
    return ref->nav2d;
}

void Terrain2D::setMesh(Rectf state, Sprite *sprite)
{
    SpriteRenderer* render;
    Transform* relative;
    if(sprite == nullptr)
        return;
    if(ref->root == nullptr)
    {
        ref->root = Primitive::CreateEmptyGameObject("Grounds")->transform();
        ref->root->setParent(transform(), false);
    }
    render = Primitive::CreateEmptyGameObject()->AddComponent<SpriteRenderer>();
    relative = render->transform();
    render->transform()->setParent(ref->root);
    render->setSprite(sprite);
    render->renderPresentMode = SpriteRenderPresentMode::Place;
    render->renderType = SpriteRenderType::Tile;
    render->setSize(state.getWH() * defaultPixelsPerPoint);

    Vec2 firstPoint = relative->position();

    relative->position(firstPoint+state.getXY());
}

void Terrain2D::load_from(const TerrainData &terrainData)
{
    throw Exception::ronin_implementation_error();
}

const bool Terrain2D::is_collider(const Vec2 destination)
{
    auto n = ref->nav2d->get(destination);
    return n && ref->nav2d->hasLocked(n);
}

const Vec2 Terrain2D::getSize() const
{
    return ref->size;
}

void Terrain2D::setSize(const Vec2 &value)
{
    if(ref->nav2d)
        RoninMemory::free(ref->nav2d);
    RoninMemory::alloc_self(ref->nav2d, static_cast<std::uint32_t>(value.x * 10), static_cast<std::uint32_t>(value.y * 10));
    ref->size = value;
}
Vec2 Terrain2D::get_offset()
{
    return {};
}

Rect Terrain2D::get_relative_size()
{
    Rect rect;

    if(ref->nav2d)
    {
        rect.w = ref->nav2d->getWidth();
        rect.h = ref->nav2d->getHeight();
    }

    return rect;
}

void Terrain2D::free_render_cache()
{
    // TODO: Free render cache for Terrain2D
}
