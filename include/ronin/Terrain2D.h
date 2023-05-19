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

    class RONIN_API Terrain2D : public Renderer
    {
        RoninEngine::AIPathFinder::NavMesh* navigation;

    public:
        Terrain2D();
        Terrain2D(int width, int length);
        Terrain2D(const std::string& name);
        Terrain2D(const Terrain2D& source);
        ~Terrain2D();

        RoninEngine::AIPathFinder::NavMesh* surfaceMesh();

        void load(const TerrainData& terrainData);

        const bool isCollider(const Vec2 destination);

        Vec2 get_size() override;
        Vec2 get_offset() override;
        Rect get_relative_size() override;
        void render(Rendering* render_info) override;
    };
} // namespace RoninEngine::Runtime
