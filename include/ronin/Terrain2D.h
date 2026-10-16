#pragma once

#include "Renderer.h"
#include <vector>

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
        struct T2Data *m_t2data;

        void ensure_root();
        void update_navmesh_offset();

    public:
        Terrain2D();
        Terrain2D(const std::string &name, float width = 10, float height = 10);
        Terrain2D(const Terrain2D &other);
        ~Terrain2D();

        RoninEngine::AI::NavMesh *get_navmesh2D();

        void setMesh(Rectf state, SpriteRef sprite);
        void setMesh(Rectf state, SpriteRef sprite, bool isCollider);
        void setMeshTile(int gridX, int gridY, SpriteRef sprite, Vec2 tileSize = Vec2::one, bool isCollider = false);
        void setTextureScale(Vec2 scale);
        Vec2 getTextureScale() const;

        void loadFrom(const TerrainData &terrainData, bool clearExisting = false);

        void setCollider(const Rectf &worldRect, bool state = true);
        void setCollider(const Vec2 &worldPoint, bool state = true);
        void setColliderCircle(const Vec2 &worldCenter, float radius, bool state = true);
        void clearColliders();
        void clearMesh();
        void clear();

        TransformRef getRoot() const;
        std::vector<SpriteRendererRef> getTiles() const;
        std::size_t getTileCount() const;

        void setZOrder(int zOrder);
        void setLayer(int layer);

        const bool is_collider(const Vec2 destination);

        const Vec2 getSize() const;
        void setSize(const Vec2 &value);
        Vec2 get_offset();
        Rect get_relative_size();
        void free_render_cache();
    };
} // namespace RoninEngine::Runtime
