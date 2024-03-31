
#include "ronin.h"
#include "ronin_exception.h"

namespace RoninEngine::Runtime
{
    Terrain2D::Terrain2D() : Terrain2D(100, 100)
    {
    }

    Terrain2D::Terrain2D(int width, int length) : Terrain2D(DESCRIBE_AS_MAIN_OFF(Terrain2D))
    {
        RoninMemory::alloc_self(navigation, width, length);
    }

    Terrain2D::Terrain2D(const std::string &name) : Renderer(DESCRIBE_AS_ONLY_NAME(Terrain2D))
    {
        DESCRIBE_AS_MAIN(Terrain2D);
    }

    Terrain2D::Terrain2D(const Terrain2D &source)
    {
        RoninMemory::alloc_self(navigation, source.navigation->getWidth(), source.navigation->getHeight());
    }
    Terrain2D::~Terrain2D()
    {
        RoninMemory::free(navigation);
    }

    AI::NavMesh *Terrain2D::get_navmesh2D()
    {
        return this->navigation;
    }

    void Terrain2D::load_from(const TerrainData &terrainData)
    {
        throw Exception::ronin_implementation_error();
    }

    const bool Terrain2D::is_collider(const Vec2 destination)
    {
        auto n = this->navigation->get(destination);
        return n && navigation->hasLocked(navigation->getPoint(n));
    }

    const Vec2 Terrain2D::getSize() const
    {
        Vec2 sz;
        throw Exception::ronin_implementation_error();
        return sz;
    }

    void Terrain2D::setSize(const Vec2 &value)
    {
        throw Exception::ronin_implementation_error();
    }
    Vec2 Terrain2D::get_offset()
    {
        return {};
    }

    Rect Terrain2D::get_relative_size()
    {
        Rect rect;

        if(navigation)
        {
            rect.w = navigation->getWidth();
            rect.h = navigation->getHeight();
        }

        return rect;
    }

    void Terrain2D::free_render_cache()
    {
        // TODO: Free render cache for Terrain2D
    }

    void Terrain2D::render(Rendering *render_info)
    {
    }
} // namespace RoninEngine::Runtime
