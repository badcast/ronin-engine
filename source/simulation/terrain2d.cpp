
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
        : Terrain2D(typeid(*this).name())
    {
        GC::gc_alloc_lval(surface, width, length);
    }

    Terrain2D::Terrain2D(const Terrain2D& source) { GC::gc_alloc_lval(surface, source.surface->getWidth(), source.surface->getHeight()); }
    Terrain2D::~Terrain2D() { GC::gc_free(surface); }

    AIPathFinder::NavMesh* Terrain2D::surfaceMesh() { return this->surface; }

    void Terrain2D::load(const TerrainData& terrainData) { }

    const bool Terrain2D::isCollider(const Vec2 destination)
    {
        auto n = this->surface->GetNeuron(destination);
        return n && surface->neuronLocked(surface->neuronGetPoint(n));
    }

    Vec2 Terrain2D::getSize() { return {}; }
    Vec2 Terrain2D::getOffset() { return {}; }

    Rect Terrain2D::getFactical()
    {
        Rect rect;

        if(surface){
            rect.w = surface->getWidth();
            rect.h = surface->getHeight();
        }

        return rect;
    }

    void Terrain2D::Render(Render_info* render_info) { }
} // namespace RoninEngine::Runtime
