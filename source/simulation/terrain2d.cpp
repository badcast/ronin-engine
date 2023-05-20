
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

    Terrain2D::Terrain2D(const Terrain2D& source) { RoninMemory::alloc_self(navigation, source.navigation->Width(), source.navigation->Height()); }
    Terrain2D::~Terrain2D() { RoninMemory::free(navigation); }

    AIPathFinder::NavMesh* Terrain2D::surfaceMesh() { return this->navigation; }

    void Terrain2D::load(const TerrainData& terrainData) { }

    const bool Terrain2D::isCollider(const Vec2 destination)
    {
        auto n = this->navigation->GetNeuron(destination);
        return n && navigation->neuronLocked(navigation->neuronGetPoint(n));
    }

    Vec2 Terrain2D::get_size() { return {}; }
    Vec2 Terrain2D::get_offset() { return {}; }

    Rect Terrain2D::get_relative_size()
    {
        Rect rect;

        if (navigation) {
            rect.w = navigation->Width();
            rect.h = navigation->Height();
        }

        return rect;
    }

    void Terrain2D::render(Rendering* render_info) { }
} // namespace RoninEngine::Runtime
