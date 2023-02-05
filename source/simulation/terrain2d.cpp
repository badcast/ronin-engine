
#include "ronin.h"

namespace RoninEngine::Runtime {

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
    GC::gc_alloc_lval(nav, width, length);
}

Terrain2D::Terrain2D(const Terrain2D& source) { GC::gc_alloc_lval(nav, source.nav->getWidth(), source.nav->getHeight()); }
Terrain2D::~Terrain2D()
{
    GC::gc_free(nav);
}

AIPathFinder::NavMesh* Terrain2D::surfaceMesh()
{
    return this->nav;
}

void Terrain2D::load(const TerrainData& terrainData)
{
}

const bool Terrain2D::isCollider(const Vec2 destination)
{
    auto n = this->nav->GetNeuron(destination);
    return n && nav->neuronLocked(nav->neuronGetPoint(n));
}

Vec2 Terrain2D::getSize() { return {}; }
Vec2 Terrain2D::getOffset() { return {}; }

void Terrain2D::Render(Render_info* render_info) { }
} // namespace RoninEngine::Runtime
