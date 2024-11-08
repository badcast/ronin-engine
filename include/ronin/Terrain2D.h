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
    private:
        struct T2Data *dat;

    public:
        Terrain2D();
        Terrain2D(const std::string &name, float width = 10, float height = 10);
        ~Terrain2D();

        RoninEngine::AI::NavMesh *get_navmesh2D();

        void setMesh(Rectf state, SpriteRef sprite);

        void load_from(const TerrainData &terrainData);

        const bool is_collider(const Vec2 destination);

        const Vec2 getSize() const;
        void setSize(const Vec2 &value);
        Vec2 get_offset();
        Rect get_relative_size();
        void free_render_cache();
    };
} // namespace RoninEngine::Runtime
