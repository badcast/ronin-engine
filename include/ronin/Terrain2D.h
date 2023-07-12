#pragma once

#include "Renderer.h"

namespace RoninEngine::Runtime
{

    struct TerrainRegion
    {
        Rect region;
        std::uint32_t textureIndex;
    };

    struct TerrainTextured
    {
        int atlasIndex;
        std::uint32_t regionCount;
        TerrainRegion *texturedRegion;
    };

    struct TerrainData
    {
        int atlasCount;
        Atlas *atlases;

        int dataCount;
        TerrainTextured *textured;
    };

    class RONIN_API Terrain2D : public Renderer
    {
        RoninEngine::AI::NavMesh *navigation;

    public:
        Terrain2D();
        Terrain2D(int width, int length);
        Terrain2D(const std::string &name);
        Terrain2D(const Terrain2D &source);
        ~Terrain2D();

        RoninEngine::AI::NavMesh *get_navmesh2D();

        void load_from(const TerrainData &terrainData);

        const bool is_collider(const Vec2 destination);

        Vec2 get_size();
        Vec2 get_offset();
        Rect get_relative_size();
        void free_render_cache();
        void render(Rendering *render_info);
    };
} // namespace RoninEngine::Runtime
