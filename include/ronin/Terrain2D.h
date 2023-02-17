#pragma once

#include "Renderer.h"

namespace RoninEngine::Runtime
{

    struct TerrainRegion {
        Rect region;
        std::uint32_t textureIndex;
    };

    struct TerrainTextured {
        int atlasIndex;
        std::uint32_t regionCount;
        TerrainRegion* texturedRegion;
    };

    struct TerrainData {
        int atlasCount;
        Atlas* atlases;

        int dataCount;
        TerrainTextured* textured;
    };

    class SHARK Terrain2D : public Renderer
    {
        RoninEngine::AIPathFinder::NavMesh* surface;

    public:
        Terrain2D();
        Terrain2D(const std::string& name);
        Terrain2D(int width, int length);
        Terrain2D(const Terrain2D& source);
        ~Terrain2D();

        RoninEngine::AIPathFinder::NavMesh* surfaceMesh();

        void load(const TerrainData& terrainData);

        const bool isCollider(const Vec2 destination);

        Vec2 getSize() override;
        Vec2 getOffset() override;
        Rect getFactical() override;
        void Render(Render_info* render_info) override;
    };
} // namespace RoninEngine::Runtime