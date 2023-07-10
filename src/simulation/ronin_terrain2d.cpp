
#include "ronin.h"

namespace RoninEngine::Runtime
{

    Terrain2D::Terrain2D()
        : Terrain2D(100, 100)
    {
    }

    Terrain2D::Terrain2D(int width, int length)
        : Terrain2D(DESCRIBE_AS_MAIN_OFF(Terrain2D))
    {
        RoninMemory::alloc_self(navigation, width, length);
    }

    Terrain2D::Terrain2D(const std::string& name)
        : Renderer(DESCRIBE_AS_ONLY_NAME(Terrain2D))
    {
        DESCRIBE_AS_MAIN(Terrain2D);
    }

    Terrain2D::Terrain2D(const Terrain2D& source)
    {
        RoninMemory::alloc_self(navigation, source.navigation->width(), source.navigation->height());
    }
    Terrain2D::~Terrain2D()
    {
        RoninMemory::free(navigation);
    }

    AIPathFinder::NavMesh* Terrain2D::get_navmesh2D()
    {
        return this->navigation;
    }

    void Terrain2D::load_from(const TerrainData& terrainData)
    {
    }

    const bool Terrain2D::is_collider(const Vec2 destination)
    {
        auto n = this->navigation->get_neuron(destination);
        return n && navigation->get_nlocked(navigation->get_npoint(n));
    }

    Vec2 Terrain2D::get_size()
    {
        return {};
    }
    Vec2 Terrain2D::get_offset()
    {
        return {};
    }

    Rect Terrain2D::get_relative_size()
    {
        Rect rect;

        if (navigation)
        {
            rect.w = navigation->width();
            rect.h = navigation->height();
        }

        return rect;
    }

    void Terrain2D::free_render_cache()
    {
        // TODO: Free render cache for Terrain2D
    }

    void Terrain2D::render(Rendering* render_info)
    {
    }
} // namespace RoninEngine::Runtime
