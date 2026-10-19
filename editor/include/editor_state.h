#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <ronin/framework.h>

#ifndef RONIN_EDITOR_VERSION_STRING
#define RONIN_EDITOR_VERSION_STRING "0.0.1-alpha"
#endif

namespace RoninEditor
{
    enum class GizmoToolMode
    {
        SelectMove = 0,
        Rotate = 1,
        Scale = 2,
        TerrainPaint = 3
    };

    enum class TerrainBrushTool
    {
        PaintTileMaterial = 0,
        FreehandSurfaceBrush = 1,
        BucketFill = 2,
        RectFill = 3,
        PaintCollider = 4,
        EraseCollider = 5,
        Eraser = 6
    };

    struct TerrainPaintStroke
    {
        RoninEngine::Runtime::Vec2 uvPos = {0.5f, 0.5f};
        float radiusNormalized = 0.08f;
        float radiusCells = 1.0f;
        float hardness = 0.65f;
        float opacity = 1.0f;
        int materialId = 0;
        RoninEngine::Runtime::Color color = {68, 148, 82, 230};
    };

    struct SpriteRendererData
    {
        int componentId = 0;
        uint32_t assetId = 0;
        bool enabled = false;
        int primitiveShape = 0; // 0 = Rectangle, 1 = Circle, 2 = Triangle, 3 = External Image
        RoninEngine::Runtime::Vec2 size = {1.0f, 1.0f};
        RoninEngine::Runtime::Vec2 pivot = {0.5f, 0.5f};
        RoninEngine::Runtime::Color color = {255, 255, 255, 255};
        int renderType = 0;     // 0 = Simple, 1 = Tile
        int renderOut = 0;      // 0 = Origin, 1 = Centering
        int presentMode = 0;    // 0 = Fixed, 1 = Place
        int flip = 0;           // 0 = FlipNone, 1 = FlipHorizontal, 2 = FlipVertical
        std::string spritePath;
    };

    struct Camera2DData
    {
        int componentId = 0;
        bool enabled = false;
        bool mainCamera = true;
        bool backclear = true;
        RoninEngine::Runtime::Color backcolor = {18, 22, 30, 255};
        float zoom = 1.0f;
        float zoomOut = 100.0f;
        RoninEngine::Runtime::Vec2 scale = {1.0f, 1.0f};
        int distanceEvcall = 0;
        bool visibleBorders = false;
        bool visibleGrids = false;
        bool visibleObjects = true;
        bool visibleNames = false;
        bool cullResources = true;
    };

    struct Light2DData
    {
        int componentId = 0;
        bool enabled = false;
        int lightType = 0; // 0 = SimpleLight, 1 = DirectLight, 2 = Spotlight
        float intensity = 1.0f;
        float range = 3.5f;
        float angleDegrees = 55.0f;
        RoninEngine::Runtime::Color color = {255, 220, 140, 220};
    };

    struct Terrain2DData
    {
        int componentId = 0;
        uint32_t customTextureAssetId = 0;
        bool enabled = false;
        int width = 12;
        int length = 8;
        float cellScale = 1.0f;
        RoninEngine::Runtime::Color tileColor = {46, 92, 64, 220};
        bool generateBorderColliders = true;
        std::string customTexturePath;

        std::vector<int> cellMaterials;
        std::vector<RoninEngine::Runtime::Color> cellColors;
        std::vector<bool> cellColliders;
        std::vector<TerrainPaintStroke> paintStrokes;

        void ensureGridInitialized(int defaultMaterial = 0);
    };

    struct ParticleSystemData
    {
        int componentId = 0;
        bool enabled = false;
        bool emit = true;
        bool loop = true;
        bool rotate = true;
        bool worldSpace = true;
        bool destroyAfter = false;
        bool randomDirection = true;
        bool reserving = true;

        float interval = 0.08f;
        float delay = 0.0f;
        int startWith = 3;
        int maxLimit = 64;
        float speed = 1.8f;
        float rotatePerFrame = 25.0f;
        float duration = 1.6f;
        RoninEngine::Runtime::Vec2 direction = {0.0f, 1.0f};

        int primitiveSource = 0; // 0 = Circle, 1 = Rectangle, 2 = Triangle
        RoninEngine::Runtime::Vec2 startSize = {0.35f, 0.35f};
        RoninEngine::Runtime::Vec2 endSize = {0.05f, 0.05f};
        RoninEngine::Runtime::Color startColor = {255, 180, 60, 240};
        RoninEngine::Runtime::Color endColor = {220, 40, 20, 0};
    };

    struct PhysicsControllerData
    {
        int componentId = 0;
        bool moveControllerEnabled = false;
        float moveSpeed = 3.5f;

        bool colliderEnabled = false;
        int colliderType = 0; // 0 = Box, 1 = Circle
        RoninEngine::Runtime::Vec2 colliderSize = {1.0f, 1.0f};
        float colliderRadius = 0.5f;
        int collisionLayer = 0;
    };

    struct AudioSourceData
    {
        int componentId = 0;
        uint32_t clipAssetId = 0;
        bool enabled = false;
        float volume = 0.85f;
        bool loop = true;
        bool playOnAwake = true;
        std::string clipPath;
    };

    struct MotionScriptData
    {
        int componentId = 0;
        bool enabled = false;
        RoninEngine::Runtime::Vec2 linearVelocity = {0.0f, 0.0f};
        float angularVelocity = 0.0f;
        float pulseSpeed = 0.0f;
        float pulseAmplitude = 0.0f;
    };

    struct Rigidbody2DData
    {
        int componentId = 0;
        bool enabled = false;
        int bodyType = 0; // 0 = Dynamic, 1 = Kinematic, 2 = Static
        float mass = 1.0f;
        float linearDamping = 0.0f;   // drag
        float angularDamping = 0.05f; // angular drag
        float gravityScale = 1.0f;
        bool freezeRotation = false;
        bool simulated = true;
        RoninEngine::Runtime::Vec2 velocity = {0.0f, 0.0f};
        float angularVelocity = 0.0f;

        void reset()
        {
            enabled = false;
            bodyType = 0;
            mass = 1.0f;
            linearDamping = 0.0f;
            angularDamping = 0.05f;
            gravityScale = 1.0f;
            freezeRotation = false;
            simulated = true;
            velocity = {0.0f, 0.0f};
            angularVelocity = 0.0f;
        }
    };

    struct BoxCollider2DData
    {
        int componentId = 0;
        bool enabled = false;
        RoninEngine::Runtime::Vec2 size = {1.0f, 1.0f};
        RoninEngine::Runtime::Vec2 offset = {0.0f, 0.0f};
        bool isTrigger = false;
        float density = 1.0f;
        float friction = 0.4f;
        float bounciness = 0.1f;

        void reset()
        {
            enabled = false;
            size = {1.0f, 1.0f};
            offset = {0.0f, 0.0f};
            isTrigger = false;
            density = 1.0f;
            friction = 0.4f;
            bounciness = 0.1f;
        }
    };

    struct NavMeshAgentData
    {
        int componentId = 0;
        bool enabled = false;
        float speed = 3.5f;
        float angularSpeed = 120.0f;
        float acceleration = 8.0f;
        float stoppingDistance = 0.25f;
        float radius = 0.4f;
        RoninEngine::Runtime::Vec2 destination = {0.0f, 0.0f};
        bool hasDestination = false;
        bool updateRotation = false;
        bool autoRepath = true;

        void reset()
        {
            enabled = false;
            speed = 3.5f;
            angularSpeed = 120.0f;
            acceleration = 8.0f;
            stoppingDistance = 0.25f;
            radius = 0.4f;
            destination = {0.0f, 0.0f};
            hasDestination = false;
            updateRotation = false;
            autoRepath = true;
        }
    };

    struct MultiCollider2DData
    {
        int componentId = 0;
        bool enabled = false;
        std::vector<RoninEngine::Runtime::Vec2> points;
        RoninEngine::Runtime::Vec2 offset = {0.0f, 0.0f};
        bool isTrigger = false;
        float density = 1.0f;
        float friction = 0.4f;
        float bounciness = 0.1f;
        bool isClosed = true;

        void reset()
        {
            enabled = false;
            offset = {0.0f, 0.0f};
            isTrigger = false;
            density = 1.0f;
            friction = 0.4f;
            bounciness = 0.1f;
            isClosed = true;
            points = {
                RoninEngine::Runtime::Vec2(-0.5f, -0.5f),
                RoninEngine::Runtime::Vec2( 0.5f, -0.5f),
                RoninEngine::Runtime::Vec2( 0.5f,  0.5f),
                RoninEngine::Runtime::Vec2(-0.5f,  0.5f)
            };
        }
    };

    struct EditorObject
    {
        int id = 0;
        std::string name = "GameObject";
        std::string tag = "Untagged";
        int layer = 0;
        bool active = true;
        int parentId = -1;

        RoninEngine::Runtime::Vec2 position = {0.0f, 0.0f};
        float angle = 0.0f;
        RoninEngine::Runtime::Vec2 scale = {1.0f, 1.0f};
        int zOrder = 0;

        SpriteRendererData sprite;
        Camera2DData camera;
        Light2DData light;
        Terrain2DData terrain;
        ParticleSystemData particles;
        PhysicsControllerData physics;
        Rigidbody2DData rigidbody2D;
        BoxCollider2DData boxCollider2D;
        MultiCollider2DData multiCollider2D;
        NavMeshAgentData navMeshAgent;
        AudioSourceData audio;
        MotionScriptData script;
    };

    // Represents a RoninEngine::UI::GUI widget inside a World
    struct EditorUIElement
    {
        int id = 0;
        std::string name = "UIElement";
        int type = 0; // 0 = Label, 1 = Button, 2 = CheckBox, 3 = Slider, 4 = TextEdit, 5 = PictureBox, 6 = DropDown
        RoninEngine::Runtime::Rect rect = {24, 24, 220, 34};
        std::string text = "UI Text";
        int fontSize = 13;
        bool visible = true;
        bool interactable = true;

        bool boolValue = false;
        float floatValue = 0.75f;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        int selectedIndex = 0;
        std::string itemsCsv = "Normal,Hard,Ronin";
        std::string targetWorldOnClick;
    };

    struct NavMeshPreviewData
    {
        int width = 16;
        int height = 10;
        int heuristicMethod = 0; // 0 = Pythagorean, 1 = Manhattan, 2 = Chebyshev, 3 = Octile
        int identityMethod = 0;  // 0 = Diagonal, 1 = Plus, 2 = Cross
        int mazeAlgorithm = 0;   // MazeAlgorithm enum (0..12)
        RoninEngine::Runtime::Vec2Int start = {1, 1};
        RoninEngine::Runtime::Vec2Int goal = {14, 8};
        std::vector<bool> lockedCells;
        std::vector<RoninEngine::Runtime::Vec2Int> path;
        bool pathFound = false;
    };

    // Simulated particle instance for live editor & GameView preview
    struct SimulatedParticle
    {
        int ownerObjectId = 0;
        RoninEngine::Runtime::Vec2 worldPos = {0.0f, 0.0f};
        RoninEngine::Runtime::Vec2 velocity = {0.0f, 0.0f};
        float angle = 0.0f;
        float angularVel = 0.0f;
        float age = 0.0f;
        float lifetime = 1.0f;
        int primitiveShape = 0;
        RoninEngine::Runtime::Vec2 startSize = {0.3f, 0.3f};
        RoninEngine::Runtime::Vec2 endSize = {0.05f, 0.05f};
        RoninEngine::Runtime::Color startColor = {255, 180, 60, 240};
        RoninEngine::Runtime::Color endColor = {220, 40, 20, 0};
    };

    // A RoninEngine::Runtime::World definition inside the Project
    struct EditorWorld
    {
        std::string name = "MainWorld";
        RoninEngine::Runtime::Color ambientColor = {24, 28, 38, 255};
        std::vector<EditorObject> objects;
        std::vector<EditorUIElement> guiElements;
        NavMeshPreviewData navMesh;
        int nextObjectId = 1;
        int nextUIElementId = 1;
        int nextComponentId = 1000;

        int allocateObjectId() { return nextObjectId++; }
        int allocateComponentId() { return nextComponentId++; }
    };

    // Single Project holding project settings + multiple Worlds
    struct EditorProject
    {
        std::string projectName = "RoninGameProject";
        std::string author = "Ronin Developer";
        std::string projectFilePath;

        int resolutionWidth = 1280;
        int resolutionHeight = 720;
        int refreshRate = 60;
        bool fullscreen = false;
        bool verticalSync = true;
        int renderBackend = 0;   // 0 = GPU, 1 = CPU
        int textureQuality = 0;  // 0 = Nearest, 1 = Linear
        float brightness = 1.0f;
        float windowOpacity = 1.0f;
        bool debugMode = false;

        float musicVolume = 0.8f;
        bool musicLoop = true;
        std::string musicClipPath;

        int startupWorldIndex = 0;
        std::vector<EditorWorld> worlds;
    };

    struct EditorLogEntry
    {
        std::string timestamp;
        std::string level;
        std::string message;
    };

    enum class EditorPlayState
    {
        Stopped = 0,
        Playing = 1,
        Paused = 2
    };

    class EditorState
    {
    public:
        EditorProject project;
        int activeWorldIndex = 0;
        bool isDirty = false;

        // Snapshot of the entire project before entering Play Mode
        EditorProject savedPlayProjectSnapshot;
        int savedActiveWorldIndex = 0;

        int selectedObjectId = -1;
        std::vector<int> selectedObjectIds;
        int selectedUIElementId = -1;

        bool isObjectSelected(int id) const;
        void selectObject(int id, bool addToSelection = false);
        void deselectObject(int id);
        void clearSelection();
        void deleteSelectedObjects();

        // Selected Asset inspection in Inspector panel
        std::filesystem::path selectedAssetPath;
        std::string selectedAssetGuid;
        bool inspectAssetMode = false;

        void selectAsset(const std::filesystem::path &path, const std::string &guid = "")
        {
            selectedAssetPath = path;
            selectedAssetGuid = guid;
            inspectAssetMode = true;
        }

        void clearAssetSelection()
        {
            selectedAssetPath.clear();
            selectedAssetGuid.clear();
            inspectAssetMode = false;
        }

        EditorPlayState playState = EditorPlayState::Stopped;
        GizmoToolMode gizmoTool = GizmoToolMode::SelectMove;
        float simTime = 0.0f;
        int simFrames = 0;
        float emitAccumulator = 0.0f;
        std::vector<SimulatedParticle> activeParticles;

        // Editor World Viewport settings
        RoninEngine::Runtime::Vec2 cameraPan = {0.0f, 0.0f};
        float pixelsPerUnit = 64.0f;
        float viewportZoom = 1.0f;
        bool showGrid = true;
        bool showGizmos = true;
        bool showBounds = true;
        bool showNavMeshOverlay = false;
        bool snapToGrid = false;
        float gridStep = 0.5f;

        // GameView settings
        bool showGameViewStats = true;
        bool showGameViewGUI = true;
        float gameViewLastFps = 60.0f;

        // Atlas slicer tool state
        int atlasImageWidth = 512;
        int atlasImageHeight = 512;
        int atlasColumns = 4;
        int atlasRows = 4;
        std::vector<RoninEngine::Runtime::Rectf> slicedUVs;

        // Physics2D query preview state
        RoninEngine::Runtime::Vec2 stormCastOrigin = {0.0f, 0.0f};
        float stormCastRadius = 3.0f;
        std::vector<int> stormCastHits;

        // Terrain 2D Editor tool state
        TerrainBrushTool terrainBrushTool = TerrainBrushTool::PaintTileMaterial;
        int terrainActiveMaterial = 0; // 0=Grass, 1=Dirt, 2=Stone, 3=Sand, 4=Water, 5=Cobble, 6=Snow, 7=Lava, 8=Wood/Tatami, 9=Custom
        float terrainBrushRadius = 0.85f;
        float terrainBrushOpacity = 0.90f;
        float terrainBrushHardness = 0.65f;
        RoninEngine::Runtime::Color terrainBrushColor = {68, 148, 82, 230};
        bool terrainShowGrid = true;
        bool terrainShowColliders = true;
        bool terrainShowStrokes = true;
        float terrainCanvasZoom = 1.0f;
        bool isTerrainEditorOpen = true;

        // MultiCollider2D vertex editing state
        bool editColliderVertices = false;
        int selectedColliderVertexIdx = -1;

        // Logs
        std::vector<EditorLogEntry> logs;

        EditorState();

        // Project & World JSON serialization (via nlohmann/json.hpp)
        void resetDefaultProject();
        bool saveProjectToFile(const std::string &filePath);
        bool loadProjectFromFile(const std::string &filePath);
        bool saveWorldToFile(const std::string &filePath, int worldIdx = -1);
        bool loadWorldFromFile(const std::string &filePath);

        // Prefab management
        bool savePrefab(const EditorObject &obj, const std::string &filePath);
        EditorObject *instantiatePrefab(const std::string &filePath, const RoninEngine::Runtime::Vec2 &spawnPos = {0.0f, 0.0f}, int parentId = -1);

        // Terrain asset management
        bool saveTerrainAsset(const Terrain2DData &terrain, const std::string &filePath);
        bool loadTerrainAsset(Terrain2DData &terrain, const std::string &filePath);

        // World management
        EditorWorld &activeWorld();
        const EditorWorld &activeWorld() const;
        int createWorld(const std::string &worldName);
        int duplicateWorld(int worldIdx);
        bool deleteWorld(int worldIdx);
        bool selectWorld(int worldIdx);
        int findWorldIndexByName(const std::string &worldName) const;

        // GameObject management in active World
        EditorObject &createObject(const std::string &name, int parentId = -1);
        void ensureComponentIds(EditorObject &obj);
        EditorObject *findObject(int id);
        const EditorObject *findObject(int id) const;
        int duplicateObject(int id);
        bool deleteObject(int id);

        // Hierarchy & Transform helpers
        RoninEngine::Runtime::Vec2 getObjectWorldPosition(const EditorObject &obj) const;
        float getObjectWorldAngle(const EditorObject &obj) const;
        RoninEngine::Runtime::Vec2 getObjectWorldScale(const EditorObject &obj) const;
        const EditorObject *findMainCameraObject() const;

        int pickObjectAt(const RoninEngine::Runtime::Vec2 &worldPos) const;
        RoninEngine::Runtime::Rectf getObjectWorldBounds(const EditorObject &obj) const;

        // Terrain 2D Editor management
        EditorObject *findActiveTerrainObject();
        const EditorObject *findActiveTerrainObject() const;
        static RoninEngine::Runtime::Color getTerrainMaterialBaseColor(int materialId, const RoninEngine::Runtime::Color &customFallback = {46, 92, 64, 220});
        static const char *getTerrainMaterialName(int materialId);
        bool paintTerrainAtUV(Terrain2DData &terrain, const RoninEngine::Runtime::Vec2 &uvPos, bool isDragContinue = false);
        void fillTerrainAll(Terrain2DData &terrain, int materialId, const RoninEngine::Runtime::Color &color);
        void floodFillTerrainCell(Terrain2DData &terrain, int startCol, int startRow, int newMaterial, const RoninEngine::Runtime::Color &newColor);
        void generateProceduralTerrain(Terrain2DData &terrain, int preset = 0);
        void applyTerrainBorderColliders(Terrain2DData &terrain, bool state = true);

        // UI::GUI Element management in active World
        EditorUIElement &createUIElement(const std::string &name, int type = 1);
        EditorUIElement *findUIElement(int id);
        const EditorUIElement *findUIElement(int id) const;
        bool deleteUIElement(int id);
        int pickUIElementAt(int screenX, int screenY, int viewW, int viewH) const;

        // Simulation & Tools
        void setPlayState(EditorPlayState newState);
        void stepSimulation(float dt);
        void runPhysicsStormCast();

        void updateAtlasSlicer();
        void updateNavMeshPreview(bool regenerateObstacles = false, bool generateMaze = false);

        void logInfo(const std::string &msg);
        void logWarn(const std::string &msg);
        void logError(const std::string &msg);
        void clearLogs();

        // Undo / Redo History
        struct HistoryRecord
        {
            std::string actionName;
            EditorWorld worldState;
            int selectedObjectId = -1;
            std::vector<int> selectedObjectIds;
        };
        std::vector<HistoryRecord> undoStack;
        std::vector<HistoryRecord> redoStack;
        static constexpr size_t kMaxHistoryRecords = 40;

        void recordUndo(const std::string &actionName);
        bool undo();
        bool redo();
        bool canUndo() const { return !undoStack.empty(); }
        bool canRedo() const { return !redoStack.empty(); }
        void clearHistory();

        static std::string getEditorVersion();
        static std::string getEngineVersion();
        static std::string getLinkedTargets();
    };
} // namespace RoninEditor
