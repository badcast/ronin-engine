#pragma once

#include "Renderer.h"
#include <vector>

namespace RoninEngine::AI
{
    class NavMesh;
    enum class MazeAlgorithm;
}

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
        SpriteRendererRef getMeshTile(int gridX, int gridY) const;
        bool removeMeshTile(int gridX, int gridY, Vec2 tileSize = Vec2::one);
        void fillMeshTiles(int startX, int startY, int width, int height, SpriteRef sprite, Vec2 tileSize = Vec2::one, bool isCollider = false);
        void setTextureScale(Vec2 scale);
        Vec2 getTextureScale() const;

        // Surface Canvas & Texture Painting API
        void createSurface(int pixelWidth = 0, int pixelHeight = 0, Color clearColor = Color(0, 0, 0, 0));
        Image *getSurface() const;
        SpriteRef getSurfaceSprite();
        bool hasSurface() const;
        void clearSurface(Color clearColor = Color(0, 0, 0, 0));
        void fillSurface(Color color);
        void paintPixel(int px, int py, Color color, bool blend = true);
        Color getPixel(int px, int py) const;
        void paintRect(Rectf localRect, Color color, bool blend = true);
        void paintCircle(Vec2 localPos, float radiusWorld, Color color, bool blend = true);
        void paintBrush(Vec2 localPos, float radiusWorld, Color color, float hardness = 0.7f);
        void paintLine(Vec2 localFrom, Vec2 localTo, float radiusWorld, Color color, float hardness = 0.7f);
        void paintTexture(Rectf localRect, SpriteRef textureSprite, Color tint = Color::white, bool tileRepeat = true);
        void paintBrushTexture(Vec2 localPos, float radiusWorld, SpriteRef textureSprite, Color tint = Color::white, float hardness = 0.7f);
        void eraseBrush(Vec2 localPos, float radiusWorld, float hardness = 0.8f);
        native_texture_t *get_render_texture();

        void loadFrom(const TerrainData &terrainData, bool clearExisting = false);

        void setCollider(const Rectf &worldRect, bool state = true);
        void setCollider(const Vec2 &worldPoint, bool state = true);
        void setColliderCircle(const Vec2 &worldCenter, float radius, bool state = true);
        void setColliderTile(int gridX, int gridY, bool state = true, Vec2 tileSize = Vec2::one);
        void setBorderColliders(bool state = true, float thickness = 0.5f);
        void clearColliders();
        void clearMesh();
        void clear();

        TransformRef getRoot() const;
        std::vector<SpriteRendererRef> getTiles() const;
        std::size_t getTileCount() const;

        void setZOrder(int zOrder);
        void setLayer(int layer);

        const bool is_collider(const Vec2 destination);
        const bool is_collider(int gridX, int gridY, Vec2 tileSize = Vec2::one);

        bool findPath(const Vec2 &startWorld, const Vec2 &goalWorld, std::vector<Vec2> &outWorldPath);
        Vec2 findFreeSpace(const Vec2 &worldOrigin, float searchRadius = 1.0f);
        void generateMaze(RoninEngine::AI::MazeAlgorithm algorithm = static_cast<RoninEngine::AI::MazeAlgorithm>(0));

        const Vec2 getSize() const;
        void setSize(const Vec2 &value);
        Vec2 get_offset();
        Rect get_relative_size();
        void free_render_cache();
    };
} // namespace RoninEngine::Runtime
