
#include "ronin.h"

namespace RoninEngine::Runtime
{

    Terrain2D::Terrain2D()
        : Terrain2D(100, 100)
    {
    }

    Terrain2D::Terrain2D(const std::string& name)
        : Renderer(name)
    {
    }

    Terrain2D::Terrain2D(int width, int length)
        : Terrain2D(DESCRIBE_TYPE(Terrain2D))
    {
        RoninMemory::alloc_self(navigation, width, length);
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

    Vec2 Terrain2D::getSize() { return {}; }
    Vec2 Terrain2D::getOffset() { return {}; }

    Rect Terrain2D::getFactical()
    {
        Rect rect;

        if (navigation) {
            rect.w = navigation->Width();
            rect.h = navigation->Height();
        }

        return rect;
    }

    void Terrain2D::Render(Render_info* render_info) { }
} // namespace RoninEngine::Runtime
