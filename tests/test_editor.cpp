#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <SimpleJSON/json.hpp>
#include "project_manager.h"
#include "editor_state.h"
#include "meta_manager.h"
#include "asset_database.h"
#include <ronin/AudioClip.h>
#include <ronin/TextStream.h>

using namespace RoninEditor;

int main()
{
    std::cout << "=== Running test_editor (ProjectManager & Structure Test) ===" << std::endl;

    std::filesystem::path testParentDir = std::filesystem::temp_directory_path() / "ronin_editor_unit_tests";
    std::string testProjectName = "TestEngineGame";
    std::filesystem::path expectedRoot = testParentDir / testProjectName;

    std::error_code ec;
    std::filesystem::remove_all(testParentDir, ec);

    ProjectManager &pm = ProjectManager::GetInstance();

    // 1. Create Project
    std::string err;
    bool created = pm.CreateProject(testParentDir.string(), testProjectName, ProjectTemplate::Action2D, err);
    if (!created)
    {
        std::cerr << "CreateProject failed: " << err << std::endl;
        return 1;
    }
    assert(created);
    assert(pm.HasOpenProject());
    assert(pm.GetCurrentProject().projectName == testProjectName);

    // 2. Verify Directory Structure
    assert(std::filesystem::exists(expectedRoot));
    assert(std::filesystem::is_directory(expectedRoot / "Assets"));
    assert(std::filesystem::is_directory(expectedRoot / "Assets" / "Worlds"));
    assert(std::filesystem::is_directory(expectedRoot / "Assets" / "Scenes"));
    assert(std::filesystem::is_directory(expectedRoot / "Assets" / "Textures"));
    assert(std::filesystem::is_directory(expectedRoot / "Assets" / "Audio"));
    assert(std::filesystem::is_directory(expectedRoot / "Assets" / "Fonts"));
    assert(std::filesystem::is_directory(expectedRoot / "Assets" / "Prefabs"));
    assert(std::filesystem::is_directory(expectedRoot / "Scripts"));
    assert(std::filesystem::is_directory(expectedRoot / "ProjectSettings"));
    assert(std::filesystem::is_directory(expectedRoot / "Library"));
    assert(std::filesystem::is_regular_file(expectedRoot / "project.ronin"));
    std::cout << "  [PASS] Directory structure verified (Assets, Scripts, ProjectSettings, Library, project.ronin)" << std::endl;

    // 3. Verify project.ronin JSON
    {
        std::ifstream ifs(expectedRoot / "project.ronin");
        assert(ifs.is_open());
        std::stringstream ss;
        ss << ifs.rdbuf();
        json::JSON pJson = json::JSON::Load(ss.str());
        assert(!pJson.IsNull());
        assert(pJson["projectName"].ToString() == testProjectName);
        assert(pJson.hasKey("projectGuid"));
        assert(!pJson["projectGuid"].ToString().empty());
        assert(pJson["formatVersion"].ToInt() == 1);
    }
    std::cout << "  [PASS] project.ronin validated" << std::endl;

    // 4. Verify ProjectSettings/ProjectSettings.json
    {
        std::ifstream ifs(expectedRoot / "ProjectSettings" / "ProjectSettings.json");
        assert(ifs.is_open());
        std::stringstream ss;
        ss << ifs.rdbuf();
        json::JSON sJson = json::JSON::Load(ss.str());
        assert(!sJson.IsNull());
        assert(sJson["projectName"].ToString() == testProjectName);
        assert(sJson["resolution"]["defaultWidth"].ToInt() == 1280);
        assert(sJson["resolution"]["defaultHeight"].ToInt() == 720);
        assert(sJson["rendering"]["backend"].ToString() == "GPU");
    }
    std::cout << "  [PASS] ProjectSettings.json validated" << std::endl;

    // 5. Verify ProjectSettings/InputSettings.json
    {
        std::ifstream ifs(expectedRoot / "ProjectSettings" / "InputSettings.json");
        assert(ifs.is_open());
        std::stringstream ss;
        ss << ifs.rdbuf();
        json::JSON iJson = json::JSON::Load(ss.str());
        assert(!iJson.IsNull());
        assert(iJson.hasKey("axes"));
        assert(iJson["axes"].size() >= 4);
    }
    std::cout << "  [PASS] InputSettings.json validated" << std::endl;

    // 6. Verify ProjectSettings/TimeSettings.json
    {
        std::ifstream ifs(expectedRoot / "ProjectSettings" / "TimeSettings.json");
        assert(ifs.is_open());
        std::stringstream ss;
        ss << ifs.rdbuf();
        json::JSON tJson = json::JSON::Load(ss.str());
        assert(!tJson.IsNull());
        assert(tJson.hasKey("fixedTimestep"));
    }
    std::cout << "  [PASS] TimeSettings.json validated" << std::endl;

    // 7. Verify Scripts/CMakeLists.txt & SamplePlayerController
    {
        assert(std::filesystem::exists(expectedRoot / "Scripts" / "CMakeLists.txt"));
        assert(std::filesystem::exists(expectedRoot / "Scripts" / "SamplePlayerController.h"));
        assert(std::filesystem::exists(expectedRoot / "Scripts" / "SamplePlayerController.cpp"));
    }
    std::cout << "  [PASS] Scripts scaffolding validated" << std::endl;

    // 8. Verify Assets/Scenes/SampleScene.roninscene
    {
        std::ifstream ifs(expectedRoot / "Assets" / "Scenes" / "SampleScene.roninscene");
        assert(ifs.is_open());
        std::stringstream ss;
        ss << ifs.rdbuf();
        json::JSON scJson = json::JSON::Load(ss.str());
        assert(!scJson.IsNull());
        assert(scJson["sceneName"].ToString() == "SampleScene");
        assert(scJson["objects"].size() >= 2);
    }
    std::cout << "  [PASS] SampleScene.roninscene validated" << std::endl;

    // 8b. Verify Assets/Worlds/SampleWorld.world
    {
        std::ifstream ifs(expectedRoot / "Assets" / "Worlds" / "SampleWorld.world");
        assert(ifs.is_open());
        std::stringstream ss;
        ss << ifs.rdbuf();
        json::JSON scJson = json::JSON::Load(ss.str());
        assert(!scJson.IsNull());
        assert(scJson["worldName"].ToString() == "SampleWorld" || scJson["sceneName"].ToString() == "SampleWorld");
        assert(scJson["objects"].size() >= 2);
    }
    std::cout << "  [PASS] SampleWorld.world validated" << std::endl;

    // 9. Test Close & Load
    pm.CloseProject();
    assert(!pm.HasOpenProject());

    bool loaded = pm.LoadProject(expectedRoot.string(), err);
    if (!loaded)
    {
        std::cerr << "LoadProject failed: " << err << std::endl;
        return 1;
    }
    assert(loaded);
    assert(pm.HasOpenProject());
    assert(pm.GetCurrentProject().projectName == testProjectName);
    std::cout << "  [PASS] Project reload roundtrip validated" << std::endl;

    // 10. Test Save
    assert(pm.SaveProject(err));
    std::cout << "  [PASS] Project save validated" << std::endl;

    // 11. Test Recent Projects
    const auto &recents = pm.GetRecentProjects();
    bool foundInRecents = false;
    for (const auto &r : recents)
    {
        if (r.name == testProjectName)
        {
            foundInRecents = true;
            assert(r.exists);
            break;
        }
    }
    assert(foundInRecents);
    pm.RemoveRecentProject(expectedRoot.string());
    std::cout << "  [PASS] Recent projects list management validated" << std::endl;

    // 12. Test Module 2: EditorState, Hierarchy & Component Systems
    {
        EditorState editorState;
        std::filesystem::path scenePath = expectedRoot / "Assets" / "Scenes" / "SampleScene.roninscene";
        assert(std::filesystem::exists(scenePath));
        assert(editorState.loadWorldFromFile(scenePath.string()));
        auto &world = editorState.activeWorld();
        assert(world.objects.size() >= 4);

        // Verify hierarchy creation & parenting
        auto &parentObj = editorState.createObject("PlayerRoot");
        parentObj.position = {1.0f, 2.0f};
        auto &childObj = editorState.createObject("SwordWeapon", parentObj.id);
        childObj.position = {0.5f, 0.0f};
        assert(childObj.parentId == parentObj.id);

        // Verify world coordinates computation
        auto worldChildPos = editorState.getObjectWorldPosition(childObj);
        assert(std::abs(worldChildPos.x - 1.5f) < 0.001f);
        assert(std::abs(worldChildPos.y - 2.0f) < 0.001f);

        // Verify picking object at world coordinates
        int pickedId = editorState.pickObjectAt({1.5f, 2.0f});
        assert(pickedId == childObj.id || pickedId == parentObj.id);

        // Verify component attachment
        childObj.sprite.enabled = true;
        childObj.sprite.size = {0.2f, 0.8f};
        childObj.sprite.color = RoninEngine::Runtime::Color{200, 200, 255, 255};
        assert(childObj.sprite.enabled);

        // Verify object duplication
        int dupId = editorState.duplicateObject(childObj.id);
        assert(dupId != -1);
        EditorObject *dupObj = editorState.findObject(dupId);
        assert(dupObj != nullptr);
        assert(dupObj->name == "SwordWeapon (Copy)");
        assert(dupObj->parentId == parentObj.id);

        // Verify object deletion
        assert(editorState.deleteObject(dupId));
        assert(editorState.findObject(dupId) == nullptr);

        std::cout << "  [PASS] Module 2: Hierarchy, Transform, Components & Picking validated" << std::endl;

        // 13. Test Terrain2D Editor & Surface Painting
        {
            auto &terrainObj = editorState.createObject("CustomTerrain");
            terrainObj.terrain.enabled = true;
            terrainObj.terrain.width = 16;
            terrainObj.terrain.length = 8;
            terrainObj.terrain.cellScale = 1.0f;
            terrainObj.terrain.ensureGridInitialized(0);

            assert(terrainObj.terrain.cellMaterials.size() == 128);
            assert(terrainObj.terrain.cellColors.size() == 128);
            assert(terrainObj.terrain.cellColliders.size() == 128);

            // Test tile paint
            editorState.terrainBrushTool = TerrainBrushTool::PaintTileMaterial;
            editorState.terrainActiveMaterial = 1; // Dirt
            editorState.terrainBrushRadius = 1.0f;
            bool paintedTile = editorState.paintTerrainAtUV(terrainObj.terrain, {0.5f, 0.5f}, false);
            assert(paintedTile);

            // Test freehand stroke paint
            editorState.terrainBrushTool = TerrainBrushTool::FreehandSurfaceBrush;
            editorState.terrainActiveMaterial = 0; // Grass
            editorState.terrainBrushRadius = 2.0f;
            bool paintedStroke = editorState.paintTerrainAtUV(terrainObj.terrain, {0.25f, 0.25f}, false);
            assert(paintedStroke);
            assert(!terrainObj.terrain.paintStrokes.empty());

            // Test fill all
            editorState.fillTerrainAll(terrainObj.terrain, 2, RoninEngine::Runtime::Color{120, 120, 120, 255});
            assert(terrainObj.terrain.cellMaterials[0] == 2);

            // Test border colliders
            editorState.applyTerrainBorderColliders(terrainObj.terrain, true);
            assert(terrainObj.terrain.cellColliders[0] == true);

            // Test scene save & reload roundtrip with terrain
            std::filesystem::path tempScene = expectedRoot / "Assets" / "Scenes" / "TerrainScene.roninscene";
            assert(editorState.saveWorldToFile(tempScene.string()));

            EditorState reloadState;
            assert(reloadState.loadWorldFromFile(tempScene.string()));
            EditorObject *loadedTerrain = reloadState.findObject(terrainObj.id);
            assert(loadedTerrain != nullptr);
            assert(loadedTerrain->terrain.enabled);
            assert(loadedTerrain->terrain.width == 16);
            assert(loadedTerrain->terrain.length == 8);
            assert(!loadedTerrain->terrain.paintStrokes.empty());
            assert(loadedTerrain->terrain.cellMaterials[0] == 2);
            assert(loadedTerrain->terrain.cellColliders[0] == true);

            std::cout << "  [PASS] Terrain2D Editor: Grid, Painting, Colliders & Serialization validated" << std::endl;
        }

        // 14. Test Multi-Selection
        {
            int idA = editorState.createObject("MultiA").id;
            int idB = editorState.createObject("MultiB").id;
            editorState.selectObject(idA, false);
            assert(editorState.isObjectSelected(idA));
            assert(!editorState.isObjectSelected(idB));

            // Ctrl+Click toggle multi-selection
            editorState.selectObject(idB, true);
            assert(editorState.isObjectSelected(idA));
            assert(editorState.isObjectSelected(idB));
            assert(editorState.selectedObjectIds.size() == 2);
            std::cout << "  [PASS] Multi-selection validated" << std::endl;
        }

        // 15. Test Rotated Object Picking
        {
            auto &rotObj = editorState.createObject("RotatedBox");
            rotObj.position = {10.0f, 10.0f};
            rotObj.angle = 45.0f; // 45 degrees
            rotObj.scale = {2.0f, 1.0f};
            rotObj.sprite.enabled = true;
            rotObj.sprite.size = {2.0f, 1.0f};

            // Point (10, 10) is center, must be picked
            int centerPick = editorState.pickObjectAt({10.0f, 10.0f});
            assert(centerPick == rotObj.id);

            // In local space, (0.8, 0) is inside box [-1, 1]. Rotated 45 deg, world pos is (10 + 0.8*cos(45), 10 + 0.8*sin(45))
            float rad = 45.0f * 3.14159265f / 180.0f;
            float px = 10.0f + 0.8f * std::cos(rad);
            float py = 10.0f + 0.8f * std::sin(rad);
            int rotatedPick = editorState.pickObjectAt({px, py});
            assert(rotatedPick == rotObj.id);
            std::cout << "  [PASS] Rotated object picking validated" << std::endl;
        }

        // 16. Test Rigidbody2D, BoxCollider2D & Physics Simulation
        {
            auto &physObj = editorState.createObject("PhysicsFallingBox");
            physObj.position = {0.0f, 10.0f};
            physObj.rigidbody2D.enabled = true;
            physObj.rigidbody2D.bodyType = 0; // Dynamic
            physObj.rigidbody2D.mass = 3.5f;
            physObj.rigidbody2D.gravityScale = 1.0f;
            physObj.boxCollider2D.enabled = true;
            physObj.boxCollider2D.size = {1.5f, 1.5f};

            // Test serialization and deserialization
            std::filesystem::path physScene = expectedRoot / "Assets" / "Scenes" / "PhysicsScene.roninscene";
            assert(editorState.saveWorldToFile(physScene.string()));

            EditorState physReload;
            assert(physReload.loadWorldFromFile(physScene.string()));
            EditorObject *loadedPhys = physReload.findObject(physObj.id);
            assert(loadedPhys != nullptr);
            assert(loadedPhys->rigidbody2D.enabled);
            assert(loadedPhys->rigidbody2D.bodyType == 0);
            assert(std::abs(loadedPhys->rigidbody2D.mass - 3.5f) < 0.001f);
            assert(loadedPhys->boxCollider2D.enabled);
            assert(std::abs(loadedPhys->boxCollider2D.size.x - 1.5f) < 0.001f);
            assert(std::abs(loadedPhys->boxCollider2D.size.y - 1.5f) < 0.001f);

            // Step simulation: object should fall under gravity (pos.y decreases)
            float initialY = loadedPhys->position.y;
            physReload.stepSimulation(0.2f);
            assert(loadedPhys->position.y < initialY);
        }

        // 17. Test Component IDs & Asset IDs
        {
            auto &obj = editorState.createObject("AssetComponentTestObj");
            obj.sprite.enabled = true;
            obj.sprite.componentId = editorState.activeWorld().allocateComponentId();
            obj.sprite.spritePath = "Assets/Sprites/Hero.sprite";
            obj.sprite.assetId = 0xA1B2C3D4;

            obj.audio.enabled = true;
            obj.audio.componentId = editorState.activeWorld().allocateComponentId();
            obj.audio.clipPath = "Assets/Audio/Slash.wav";
            obj.audio.clipAssetId = 0x12345678;

            std::filesystem::path idWorldPath = expectedRoot / "Assets" / "Worlds" / "IdTestWorld.world";
            assert(editorState.saveWorldToFile(idWorldPath.string()));

            EditorState idReload;
            assert(idReload.loadWorldFromFile(idWorldPath.string()));
            EditorObject *loadedObj = idReload.findObject(obj.id);
            assert(loadedObj != nullptr);
            assert(loadedObj->sprite.enabled);
            assert(loadedObj->sprite.componentId == obj.sprite.componentId);
            assert(loadedObj->sprite.assetId == 0xA1B2C3D4);
            assert(loadedObj->audio.enabled);
            assert(loadedObj->audio.componentId == obj.audio.componentId);
            assert(loadedObj->audio.clipAssetId == 0x12345678);
            std::cout << "  [PASS] Component IDs & Asset IDs persisted and validated" << std::endl;
        }

        // 18. Test Recursive Duplication & Unique Component IDs
        {
            int parentId = editorState.createObject("ParentHier").id;
            EditorObject *pObj = editorState.findObject(parentId);
            assert(pObj != nullptr);
            pObj->sprite.enabled = true;
            pObj->sprite.componentId = editorState.activeWorld().allocateComponentId();

            int childId = editorState.createObject("ChildHier", parentId).id;
            EditorObject *cObj = editorState.findObject(childId);
            assert(cObj != nullptr);
            cObj->camera.enabled = true;
            cObj->camera.componentId = editorState.activeWorld().allocateComponentId();

            int dupParentId = editorState.duplicateObject(parentId);
            assert(dupParentId != parentId);
            EditorObject *dupParent = editorState.findObject(dupParentId);
            assert(dupParent != nullptr);
            assert(dupParent->sprite.enabled);
            assert(dupParent->sprite.componentId != pObj->sprite.componentId);

            // Find duplicated child
            int dupChildId = -1;
            for (const auto &o : editorState.activeWorld().objects)
            {
                if (o.parentId == dupParentId)
                {
                    dupChildId = o.id;
                    break;
                }
            }
            assert(dupChildId != -1);
            EditorObject *dupChild = editorState.findObject(dupChildId);
            assert(dupChild != nullptr);
            assert(dupChild->camera.enabled);
            assert(dupChild->camera.componentId != cObj->camera.componentId);
            std::cout << "  [PASS] Recursive object duplication & unique component IDs validated" << std::endl;
        }

        // 19. Test Undo / Redo System
        {
            size_t countBefore = editorState.activeWorld().objects.size();
            int newObjId = editorState.createObject("UndoTestObj").id;
            assert(editorState.activeWorld().objects.size() == countBefore + 1);
            assert(editorState.canUndo());

            // Undo creation
            bool undoOk = editorState.undo();
            assert(undoOk);
            assert(editorState.activeWorld().objects.size() == countBefore);
            assert(editorState.findObject(newObjId) == nullptr);
            assert(editorState.canRedo());

            // Redo creation
            bool redoOk = editorState.redo();
            assert(redoOk);
            assert(editorState.activeWorld().objects.size() == countBefore + 1);
            assert(editorState.findObject(newObjId) != nullptr);

            // Undo again to leave world clean
            editorState.undo();
            assert(editorState.activeWorld().objects.size() == countBefore);
            std::cout << "  [PASS] Undo / Redo system validated" << std::endl;
        }

        // 20. Test Component Asset Assignment & Custom Image Shape
        {
            int testObjId = editorState.createObject("AssetAssignTest").id;
            EditorObject *testObj = editorState.findObject(testObjId);
            assert(testObj != nullptr);

            std::string spriteRel = "Assets/Textures/NewSprite.sprite";
            uint32_t spriteId = 0x50524954;
            testObj->sprite.enabled = true;
            testObj->sprite.spritePath = spriteRel;
            testObj->sprite.assetId = spriteId;
            testObj->sprite.primitiveShape = 3; // Custom Image

            std::string audioRel = "Assets/Audio/Project_2.ogg";
            uint32_t audioId = 0x41554449;
            testObj->audio.enabled = true;
            testObj->audio.clipPath = audioRel;
            testObj->audio.clipAssetId = audioId;

            assert(testObj->sprite.spritePath == spriteRel);
            assert(testObj->sprite.primitiveShape == 3);
            assert(testObj->audio.clipPath == audioRel);

            std::filesystem::path assetWorldPath = expectedRoot / "Assets" / "Worlds" / "AssetAssignWorld.world";
            assert(editorState.saveWorldToFile(assetWorldPath.string()));

            EditorState reloadState;
            assert(reloadState.loadWorldFromFile(assetWorldPath.string()));
            EditorObject *reloaded = reloadState.findObject(testObjId);
            assert(reloaded != nullptr);
            assert(reloaded->sprite.enabled);
            assert(reloaded->sprite.spritePath == spriteRel);
            assert(reloaded->sprite.assetId == spriteId);
            assert(reloaded->audio.enabled);
            assert(reloaded->audio.clipPath == audioRel);
            assert(reloaded->audio.clipAssetId == audioId);
            std::cout << "  [PASS] Component Asset Assignment validated" << std::endl;
        }

        // 21. Validate AudioClip engine class
        {
            RoninEngine::Runtime::AudioClip clip;
            assert(!clip.valid());
            assert(clip.getMixChunk() == nullptr);
            assert(clip.getDuration() == -1.0);
            assert(clip.getLengthInBytes() == 0);
            clip.name = "TestClip";
            clip.guid = "1234567890abcdef1234567890abcdef";
            clip.assetId = 0x12345678;
            assert(clip.name == "TestClip");
            assert(clip.guid == "1234567890abcdef1234567890abcdef");
            assert(clip.assetId == 0x12345678);
            std::cout << "  [PASS] Test 21: AudioClip engine type validated" << std::endl;
        }

        // 22. Validate TextStream engine class
        {
            std::filesystem::path txtFile = expectedRoot / "Assets" / "Scripts" / "test_stream.txt";
            std::string sampleContent = "First Line\nSecond Line\nThird Line";
            assert(RoninEngine::Runtime::TextStream::WriteAllText(txtFile.string(), sampleContent));
            assert(std::filesystem::exists(txtFile));

            std::string readBack = RoninEngine::Runtime::TextStream::ReadAllText(txtFile.string());
            assert(readBack == sampleContent);

            std::vector<std::string> lines = RoninEngine::Runtime::TextStream::ReadLines(txtFile.string());
            assert(lines.size() == 3);
            assert(lines[0] == "First Line");
            assert(lines[1] == "Second Line");
            assert(lines[2] == "Third Line");

            RoninEngine::Runtime::TextStream stream;
            assert(stream.Open(txtFile.string(), std::ios::in));
            assert(stream.IsOpen());
            assert(!stream.IsEOF());
            assert(stream.ReadLine() == "First Line");
            assert(stream.ReadLine() == "Second Line");
            assert(stream.ReadLine() == "Third Line");
            stream.Close();
            assert(!stream.IsOpen());
            std::cout << "  [PASS] Test 22: TextStream engine type validated" << std::endl;
        }

        // 23. Validate MetaManager Unity-style GUID & metadata
        {
            std::string guid1 = MetaManager::GenerateGuid();
            std::string guid2 = MetaManager::GenerateGuid();
            assert(guid1.length() == 32);
            assert(guid2.length() == 32);
            assert(guid1 != guid2);
            for (char c : guid1)
            {
                assert((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
            }

            uint32_t id1 = MetaManager::CalculateAssetId("Assets/Audio/Project_2.ogg");
            uint32_t id2 = MetaManager::CalculateAssetId("assets/audio/project_2.ogg");
            assert(id1 == id2);
            assert(id1 != 0);

            assert(MetaManager::DetermineAssetType("hero.sprite") == AssetType::Sprite);
            assert(MetaManager::DetermineAssetType("hero.png") == AssetType::Texture);
            assert(MetaManager::DetermineAssetType("sound.wav") == AssetType::Audio);
            assert(MetaManager::DetermineAssetType("level.world") == AssetType::World);
            assert(MetaManager::DetermineAssetType("player.prefab") == AssetType::Prefab);
            assert(MetaManager::DetermineAssetType("ground.terrain") == AssetType::Terrain);
            assert(MetaManager::DetermineAssetType("cutscene.mp4") == AssetType::Video);
            // Source code files strictly Code (.code for the system)
            assert(MetaManager::DetermineAssetType("PlayerController.h") == AssetType::Code);
            assert(MetaManager::DetermineAssetType("PlayerController.hpp") == AssetType::Code);
            assert(MetaManager::DetermineAssetType("system.c") == AssetType::Code);
            assert(MetaManager::DetermineAssetType("PlayerController.cpp") == AssetType::Code);
            assert(MetaManager::DetermineAssetType("math_simd.cxx") == AssetType::Code);
            assert(MetaManager::DetermineAssetType("intrinsics.inc") == AssetType::Code);

            // Text data streams strictly TextStream
            assert(MetaManager::DetermineAssetType("readme.txt") == AssetType::TextStream);
            assert(MetaManager::DetermineAssetType("dialogues.json") == AssetType::TextStream);
            assert(MetaManager::DetermineAssetType("engine.log") == AssetType::TextStream);

            std::filesystem::path dummyAsset = expectedRoot / "Assets" / "Textures" / "Hero.png";
            std::filesystem::create_directories(dummyAsset.parent_path(), ec);
            std::ofstream(dummyAsset) << "fake_png_data";

            MetaFile metaOut;
            assert(MetaManager::EnsureMetaFile(dummyAsset, metaOut));
            assert(metaOut.type == AssetType::Texture);
            assert(metaOut.guid.length() == 32);
            assert(std::filesystem::exists(dummyAsset.string() + ".meta"));

            // Reload and verify persistence
            MetaFile reloadedMeta;
            assert(MetaManager::EnsureMetaFile(dummyAsset, reloadedMeta));
            assert(reloadedMeta.guid == metaOut.guid);
            assert(reloadedMeta.assetId == metaOut.assetId);
            std::cout << "  [PASS] Test 23: MetaManager Unity-style GUID and metadata validated" << std::endl;
        }

        // 24. Validate AssetDatabase pipeline & stem resolution
        {
            std::filesystem::path audioAsset = expectedRoot / "Assets" / "Audio" / "Project_2.ogg";
            std::filesystem::create_directories(audioAsset.parent_path(), ec);
            std::ofstream(audioAsset) << "fake_ogg_data";

            std::filesystem::path spriteAsset = expectedRoot / "Assets" / "Textures" / "Warrior.sprite";
            std::ofstream(spriteAsset) << "{\"name\":\"Warrior\"}";

            AssetDatabase &adb = AssetDatabase::GetInstance();
            adb.Initialize(expectedRoot);

            // Verify .meta companion creation
            assert(std::filesystem::exists(audioAsset.string() + ".meta"));
            assert(std::filesystem::exists(spriteAsset.string() + ".meta"));

            // Verify stem resolution (without extensions)
            assert(adb.GetStemByPath(audioAsset) == "Project_2");
            assert(adb.GetStemByPath(spriteAsset) == "Warrior");

            // Verify lookups by path and by guid
            std::string audioGuid = adb.GetGuidByPath(audioAsset);
            assert(!audioGuid.empty());
            assert(adb.HasAssetByGuid(audioGuid));
            assert(adb.GetStemByGuid(audioGuid) == "Project_2");
            assert(adb.GetPathByGuid(audioGuid) == audioAsset);

            const AssetMetadata *audioMd = adb.GetMetadataByGuid(audioGuid);
            assert(audioMd != nullptr);
            assert(audioMd->type == AssetType::Audio);
            assert(audioMd->filename == "Project_2.ogg");

            // Verify deletion removes both file and .meta
            assert(adb.DeleteAsset(spriteAsset));
            assert(!std::filesystem::exists(spriteAsset));
            assert(!std::filesystem::exists(spriteAsset.string() + ".meta"));
            std::cout << "  [PASS] Test 24: AssetDatabase pipeline and stem resolution validated" << std::endl;
        }
    }

    // Cleanup
    pm.CloseProject();
    std::filesystem::remove_all(testParentDir, ec);

    std::cout << "=== test_editor PASSED ALL CHECKS ===" << std::endl;
    return 0;
}
