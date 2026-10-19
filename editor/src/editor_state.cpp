#include "editor_state.h"
#include "meta_manager.h"
#include "asset_database.h"
#include <nlohmann/json.hpp>
#include <box2d/b2_world.h>
#include <box2d/b2_body.h>

#include <algorithm>
#include <chrono>
#include <clocale>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <utility>

using namespace RoninEngine;
using namespace RoninEngine::Runtime;

namespace RoninEditor
{
    using json = nlohmann::json;
    static std::string currentTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tmBuf{};
#if defined(_WIN32)
        localtime_s(&tmBuf, &t);
#else
        localtime_r(&t, &tmBuf);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tmBuf, "%H:%M:%S");
        return oss.str();
    }

    static void initDefaultNavMesh(NavMeshPreviewData &nav)
    {
        nav.width = 16;
        nav.height = 10;
        nav.start = Vec2Int(1, 1);
        nav.goal = Vec2Int(14, 8);
        nav.lockedCells.assign(nav.width * nav.height, false);
        int midX = nav.width / 2;
        for (int y = 1; y < nav.height - 1; ++y)
        {
            if (y != nav.height / 2 && y != (nav.height / 2) - 1)
                nav.lockedCells[y * nav.width + midX] = true;
        }
    }

    void Terrain2DData::ensureGridInitialized(int defaultMaterial)
    {
        int w = std::max(1, width);
        int l = std::max(1, length);
        width = w;
        length = l;
        size_t total = static_cast<size_t>(w * l);
        if (cellMaterials.size() != total)
            cellMaterials.resize(total, defaultMaterial);
        if (cellColors.size() != total)
            cellColors.resize(total, EditorState::getTerrainMaterialBaseColor(defaultMaterial, tileColor));
        if (cellColliders.size() != total)
        {
            cellColliders.resize(total, false);
            if (generateBorderColliders)
            {
                for (int r = 0; r < l; ++r)
                {
                    for (int c = 0; c < w; ++c)
                    {
                        if (r == 0 || r == l - 1 || c == 0 || c == w - 1)
                            cellColliders[r * w + c] = true;
                    }
                }
            }
        }
    }

    EditorState::EditorState()
    {
        resetDefaultProject();
        updateAtlasSlicer();
        updateNavMeshPreview(true, false);
        runPhysicsStormCast();
        logInfo("Ronin Editor v" + getEditorVersion() + " initialized (Engine v" + getEngineVersion() + ")");
    }

    void EditorState::resetDefaultProject()
    {
        project = EditorProject{};
        project.projectName = "RoninChronicles";
        project.author = "Ronin Studio";
        project.projectFilePath = "RoninChronicles.roninproj";
        project.resolutionWidth = 1280;
        project.resolutionHeight = 720;
        project.refreshRate = 60;
        project.startupWorldIndex = 0;
        project.worlds.clear();

        // =========================================================================
        // WORLD 1: SamuraiDojoWorld (Startup Gameplay World)
        // =========================================================================
        {
            EditorWorld w;
            w.name = "SamuraiDojoWorld";
            w.ambientColor = Color(20, 26, 36, 255);
            initDefaultNavMesh(w.navMesh);

            auto addObj = [&](const std::string &name, int parentId = -1) -> EditorObject & {
                EditorObject o;
                o.id = w.nextObjectId++;
                o.name = name;
                o.parentId = parentId;
                w.objects.push_back(o);
                return w.objects.back();
            };

            // 1. Main Camera
            {
                EditorObject &cam = addObj("Main Camera");
                cam.tag = "MainCamera";
                cam.position = Vec2(0.0f, 0.0f);
                cam.zOrder = 0;
                cam.camera.enabled = true;
                cam.camera.mainCamera = true;
                cam.camera.backclear = true;
                cam.camera.backcolor = Color(18, 24, 34, 255);
                cam.camera.zoom = 1.0f;
                cam.camera.zoomOut = 100.0f;
                cam.camera.visibleObjects = true;
                cam.camera.cullResources = true;
            }

            // 2. Ground Terrain2D
            {
                EditorObject &terrain = addObj("Dojo Ground Terrain");
                terrain.tag = "Terrain";
                terrain.position = Vec2(0.0f, 0.0f);
                terrain.zOrder = -20;
                terrain.terrain.enabled = true;
                terrain.terrain.width = 14;
                terrain.terrain.length = 9;
                terrain.terrain.cellScale = 1.0f;
                terrain.terrain.tileColor = Color(42, 85, 62, 220);
                terrain.terrain.generateBorderColliders = true;
                generateProceduralTerrain(terrain.terrain, 0);
            }

            // 3. Shrine Torii + Child Roof
            int toriiId = -1;
            {
                EditorObject &torii = addObj("Shrine Torii Gate");
                toriiId = torii.id;
                torii.tag = "Environment";
                torii.position = Vec2(-3.2f, 1.4f);
                torii.zOrder = 5;
                torii.sprite.enabled = true;
                torii.sprite.primitiveShape = 0; // Rectangle
                torii.sprite.size = Vec2(2.2f, 2.5f);
                torii.sprite.pivot = Vec2(0.5f, 0.5f);
                torii.sprite.color = Color(214, 58, 46, 245);
                torii.physics.colliderEnabled = true;
                torii.physics.colliderSize = Vec2(2.2f, 2.5f);
            }
            {
                EditorObject &roof = addObj("Torii Top Beam", toriiId);
                roof.tag = "Environment";
                roof.position = Vec2(0.0f, 1.35f); // Local to Torii
                roof.zOrder = 6;
                roof.sprite.enabled = true;
                roof.sprite.primitiveShape = 0;
                roof.sprite.size = Vec2(2.8f, 0.45f);
                roof.sprite.color = Color(40, 42, 50, 255);
            }

            // 4. Player Samurai + Child Katana
            int playerId = -1;
            {
                EditorObject &player = addObj("Player Samurai");
                playerId = player.id;
                player.tag = "Player";
                player.position = Vec2(0.0f, -0.4f);
                player.zOrder = 10;
                player.sprite.enabled = true;
                player.sprite.primitiveShape = 0;
                player.sprite.size = Vec2(1.3f, 1.7f);
                player.sprite.pivot = Vec2(0.5f, 0.5f);
                player.sprite.color = Color(56, 178, 224, 255);
                player.physics.moveControllerEnabled = true;
                player.physics.moveSpeed = 4.0f;
                player.physics.colliderEnabled = true;
                player.physics.colliderSize = Vec2(1.3f, 1.7f);
                player.script.enabled = true;
                player.script.linearVelocity = Vec2(1.4f, 0.0f);
                player.script.pulseSpeed = 2.5f;
                player.script.pulseAmplitude = 0.45f;
            }
            {
                EditorObject &katana = addObj("Katana Blade", playerId);
                katana.tag = "Weapon";
                katana.position = Vec2(0.85f, 0.15f); // Local offset from Player
                katana.angle = 25.0f;
                katana.zOrder = 11;
                katana.sprite.enabled = true;
                katana.sprite.primitiveShape = 2; // Triangle blade
                katana.sprite.size = Vec2(0.95f, 0.28f);
                katana.sprite.color = Color(230, 240, 255, 255);
                katana.script.enabled = true;
                katana.script.angularVelocity = 90.0f;
            }

            // 5. Brazier Fire ParticleSystem + SimpleLight
            {
                EditorObject &brazier = addObj("Brazier Fire Emitter");
                brazier.tag = "Particles";
                brazier.position = Vec2(-3.2f, -1.6f);
                brazier.zOrder = 14;
                brazier.particles.enabled = true;
                brazier.particles.emit = true;
                brazier.particles.loop = true;
                brazier.particles.interval = 0.06f;
                brazier.particles.speed = 2.0f;
                brazier.particles.duration = 1.4f;
                brazier.particles.direction = Vec2(0.0f, 1.0f);
                brazier.particles.primitiveSource = 0; // Circle
                brazier.particles.startSize = Vec2(0.38f, 0.38f);
                brazier.particles.endSize = Vec2(0.06f, 0.06f);
                brazier.particles.startColor = Color(255, 195, 65, 240);
                brazier.particles.endColor = Color(235, 45, 20, 0);

                brazier.light.enabled = true;
                brazier.light.lightType = 0; // SimpleLight
                brazier.light.intensity = 1.2f;
                brazier.light.range = 3.5f;
                brazier.light.color = Color(255, 185, 75, 210);
            }

            // 6. Watchtower Spotlight
            {
                EditorObject &spot = addObj("Watchtower Spotlight");
                spot.tag = "Light";
                spot.position = Vec2(3.2f, 1.8f);
                spot.angle = -135.0f;
                spot.zOrder = 15;
                spot.light.enabled = true;
                spot.light.lightType = 2; // Spotlight
                spot.light.intensity = 1.35f;
                spot.light.range = 5.2f;
                spot.light.angleDegrees = 50.0f;
                spot.light.color = Color(120, 215, 255, 210);
                spot.script.enabled = true;
                spot.script.angularVelocity = 35.0f;
            }

            // 7. Ambient Wind AudioSource
            {
                EditorObject &aud = addObj("Ambient Wind Audio");
                aud.tag = "Audio";
                aud.position = Vec2(3.0f, -2.0f);
                aud.zOrder = 2;
                aud.audio.enabled = true;
                aud.audio.volume = 0.85f;
                aud.audio.loop = true;
                aud.audio.playOnAwake = true;
                aud.audio.clipPath = "data/audio/wind_loop.ogg";
            }

            // UI::GUI Elements for SamuraiDojoWorld
            {
                EditorUIElement lbl;
                lbl.id = w.nextUIElementId++;
                lbl.name = "HUD World Banner";
                lbl.type = 0; // Label
                lbl.rect = Rect(20, 16, 320, 28);
                lbl.text = "WORLD: SamuraiDojoWorld (WASD / Arrows)";
                w.guiElements.push_back(lbl);

                EditorUIElement btnDungeon;
                btnDungeon.id = w.nextUIElementId++;
                btnDungeon.name = "Btn Enter Dungeon";
                btnDungeon.type = 1; // Button
                btnDungeon.rect = Rect(20, 52, 250, 32);
                btnDungeon.text = "Load DungeonBossWorld ->";
                btnDungeon.targetWorldOnClick = "DungeonBossWorld";
                w.guiElements.push_back(btnDungeon);

                EditorUIElement btnMenu;
                btnMenu.id = w.nextUIElementId++;
                btnMenu.name = "Btn Main Menu";
                btnMenu.type = 1; // Button
                btnMenu.rect = Rect(20, 92, 250, 32);
                btnMenu.text = "Load MainMenuWorld";
                btnMenu.targetWorldOnClick = "MainMenuWorld";
                w.guiElements.push_back(btnMenu);

                EditorUIElement sld;
                sld.id = w.nextUIElementId++;
                sld.name = "Ki Energy Slider";
                sld.type = 3; // Slider
                sld.rect = Rect(20, 134, 240, 28);
                sld.text = "Ki Energy";
                sld.floatValue = 0.82f;
                w.guiElements.push_back(sld);

                EditorUIElement chk;
                chk.id = w.nextUIElementId++;
                chk.name = "Stealth Mode CheckBox";
                chk.type = 2; // CheckBox
                chk.rect = Rect(20, 170, 180, 26);
                chk.text = "Shadow Cloak";
                chk.boolValue = true;
                w.guiElements.push_back(chk);
            }

            project.worlds.push_back(w);
        }

        // =========================================================================
        // WORLD 2: DungeonBossWorld (Boss Arena World)
        // =========================================================================
        {
            EditorWorld w;
            w.name = "DungeonBossWorld";
            w.ambientColor = Color(14, 10, 22, 255);
            initDefaultNavMesh(w.navMesh);

            auto addObj = [&](const std::string &name, int parentId = -1) -> EditorObject & {
                EditorObject o;
                o.id = w.nextObjectId++;
                o.name = name;
                o.parentId = parentId;
                w.objects.push_back(o);
                return w.objects.back();
            };

            // 1. Dungeon Camera
            {
                EditorObject &cam = addObj("Dungeon Camera");
                cam.tag = "MainCamera";
                cam.position = Vec2(0.0f, 0.0f);
                cam.camera.enabled = true;
                cam.camera.mainCamera = true;
                cam.camera.backclear = true;
                cam.camera.backcolor = Color(14, 10, 22, 255);
                cam.camera.zoom = 0.9f;
            }

            // 2. Obsidian Floor Terrain
            {
                EditorObject &terrain = addObj("Obsidian Arena Terrain");
                terrain.tag = "Terrain";
                terrain.zOrder = -20;
                terrain.terrain.enabled = true;
                terrain.terrain.width = 16;
                terrain.terrain.length = 10;
                terrain.terrain.cellScale = 1.0f;
                terrain.terrain.tileColor = Color(58, 28, 48, 220);
                terrain.terrain.generateBorderColliders = true;
                generateProceduralTerrain(terrain.terrain, 1);
            }

            // 3. Boss Oni Overlord
            {
                EditorObject &boss = addObj("Boss Oni Overlord");
                boss.tag = "Boss";
                boss.position = Vec2(0.0f, 0.6f);
                boss.zOrder = 12;
                boss.sprite.enabled = true;
                boss.sprite.primitiveShape = 1; // Circle
                boss.sprite.size = Vec2(2.4f, 2.4f);
                boss.sprite.color = Color(210, 45, 75, 255);
                boss.physics.colliderEnabled = true;
                boss.physics.colliderType = 1;
                boss.physics.colliderRadius = 1.2f;
                boss.script.enabled = true;
                boss.script.linearVelocity = Vec2(2.0f, 0.5f);
                boss.script.angularVelocity = 45.0f;
                boss.script.pulseSpeed = 3.2f;
                boss.script.pulseAmplitude = 0.5f;

                boss.light.enabled = true;
                boss.light.lightType = 0;
                boss.light.intensity = 1.5f;
                boss.light.range = 4.5f;
                boss.light.color = Color(255, 50, 90, 220);
            }

            // 4. Soul Vortex Emitter
            {
                EditorObject &vortex = addObj("Soul Vortex Particles");
                vortex.tag = "Particles";
                vortex.position = Vec2(0.0f, 0.6f);
                vortex.zOrder = 15;
                vortex.particles.enabled = true;
                vortex.particles.emit = true;
                vortex.particles.loop = true;
                vortex.particles.randomDirection = true;
                vortex.particles.interval = 0.05f;
                vortex.particles.speed = 2.6f;
                vortex.particles.duration = 1.5f;
                vortex.particles.primitiveSource = 2; // Triangle shards
                vortex.particles.startSize = Vec2(0.42f, 0.42f);
                vortex.particles.endSize = Vec2(0.05f, 0.05f);
                vortex.particles.startColor = Color(140, 80, 255, 240);
                vortex.particles.endColor = Color(40, 220, 255, 0);
            }

            // UI::GUI for DungeonBossWorld
            {
                EditorUIElement lbl;
                lbl.id = w.nextUIElementId++;
                lbl.name = "Boss Title Label";
                lbl.type = 0;
                lbl.rect = Rect(20, 16, 360, 28);
                lbl.text = "BOSS ARENA: ONI OVERLORD";
                w.guiElements.push_back(lbl);

                EditorUIElement hp;
                hp.id = w.nextUIElementId++;
                hp.name = "Boss HP Bar";
                hp.type = 3; // Slider
                hp.rect = Rect(20, 50, 320, 28);
                hp.text = "Oni HP";
                hp.floatValue = 0.95f;
                w.guiElements.push_back(hp);

                EditorUIElement btnBack;
                btnBack.id = w.nextUIElementId++;
                btnBack.name = "Btn Return Dojo";
                btnBack.type = 1; // Button
                btnBack.rect = Rect(20, 90, 260, 32);
                btnBack.text = "<- Return to SamuraiDojoWorld";
                btnBack.targetWorldOnClick = "SamuraiDojoWorld";
                w.guiElements.push_back(btnBack);
            }

            project.worlds.push_back(w);
        }

        // =========================================================================
        // WORLD 3: MainMenuWorld (Interactive Title Screen World)
        // =========================================================================
        {
            EditorWorld w;
            w.name = "MainMenuWorld";
            w.ambientColor = Color(10, 14, 24, 255);
            initDefaultNavMesh(w.navMesh);

            auto addObj = [&](const std::string &name, int parentId = -1) -> EditorObject & {
                EditorObject o;
                o.id = w.nextObjectId++;
                o.name = name;
                o.parentId = parentId;
                w.objects.push_back(o);
                return w.objects.back();
            };

            {
                EditorObject &cam = addObj("Menu Camera");
                cam.tag = "MainCamera";
                cam.camera.enabled = true;
                cam.camera.mainCamera = true;
                cam.camera.backclear = true;
                cam.camera.backcolor = Color(10, 14, 24, 255);
            }

            {
                EditorObject &crest = addObj("Ronin Crest Emblem");
                crest.position = Vec2(2.2f, 0.2f);
                crest.zOrder = 5;
                crest.sprite.enabled = true;
                crest.sprite.primitiveShape = 1; // Circle
                crest.sprite.size = Vec2(2.8f, 2.8f);
                crest.sprite.color = Color(220, 165, 55, 230);
                crest.script.enabled = true;
                crest.script.angularVelocity = 20.0f;
            }

            {
                EditorObject &sakura = addObj("Sakura Blossom Particles");
                sakura.position = Vec2(2.2f, 2.5f);
                sakura.zOrder = 10;
                sakura.particles.enabled = true;
                sakura.particles.emit = true;
                sakura.particles.loop = true;
                sakura.particles.interval = 0.08f;
                sakura.particles.speed = 1.4f;
                sakura.particles.direction = Vec2(-0.4f, -1.0f);
                sakura.particles.duration = 2.2f;
                sakura.particles.startSize = Vec2(0.28f, 0.28f);
                sakura.particles.endSize = Vec2(0.10f, 0.10f);
                sakura.particles.startColor = Color(255, 160, 200, 240);
                sakura.particles.endColor = Color(255, 210, 230, 0);
            }

            {
                EditorUIElement title;
                title.id = w.nextUIElementId++;
                title.name = "Main Title Label";
                title.type = 0;
                title.rect = Rect(40, 40, 380, 34);
                title.text = "RONIN CHRONICLES — ENGINE DEMO";
                title.fontSize = 16;
                w.guiElements.push_back(title);

                EditorUIElement b1;
                b1.id = w.nextUIElementId++;
                b1.name = "Btn Start Dojo";
                b1.type = 1;
                b1.rect = Rect(40, 95, 260, 36);
                b1.text = "Play SamuraiDojoWorld";
                b1.targetWorldOnClick = "SamuraiDojoWorld";
                w.guiElements.push_back(b1);

                EditorUIElement b2;
                b2.id = w.nextUIElementId++;
                b2.name = "Btn Start Boss";
                b2.type = 1;
                b2.rect = Rect(40, 142, 260, 36);
                b2.text = "Play DungeonBossWorld";
                b2.targetWorldOnClick = "DungeonBossWorld";
                w.guiElements.push_back(b2);

                EditorUIElement dd;
                dd.id = w.nextUIElementId++;
                dd.name = "Difficulty DropDown";
                dd.type = 6; // DropDown
                dd.rect = Rect(40, 192, 260, 32);
                dd.text = "Difficulty";
                dd.itemsCsv = "Ashigaru,Samurai,Ronin Master";
                dd.selectedIndex = 1;
                w.guiElements.push_back(dd);
            }

            project.worlds.push_back(w);
        }

        activeWorldIndex = 0;
        selectedObjectId = 4; // Player Samurai in SamuraiDojoWorld
        selectedUIElementId = activeWorld().guiElements.empty() ? -1 : activeWorld().guiElements.front().id;
        playState = EditorPlayState::Stopped;
        activeParticles.clear();
        simTime = 0.0f;
        simFrames = 0;
        isDirty = false;
        clearHistory();

        logInfo("Initialized Project '" + project.projectName + "' with " +
                std::to_string(project.worlds.size()) + " Worlds.");
    }

    EditorWorld &EditorState::activeWorld()
    {
        if (project.worlds.empty())
        {
            EditorWorld w;
            w.name = "DefaultWorld";
            initDefaultNavMesh(w.navMesh);
            project.worlds.push_back(w);
            activeWorldIndex = 0;
        }
        activeWorldIndex = std::clamp(activeWorldIndex, 0, static_cast<int>(project.worlds.size()) - 1);
        return project.worlds[activeWorldIndex];
    }

    const EditorWorld &EditorState::activeWorld() const
    {
        int idx = std::clamp(activeWorldIndex, 0, static_cast<int>(project.worlds.size()) - 1);
        return project.worlds[idx];
    }

    int EditorState::createWorld(const std::string &worldName)
    {
        EditorWorld w;
        w.name = worldName.empty() ? ("World_" + std::to_string(project.worlds.size() + 1)) : worldName;
        initDefaultNavMesh(w.navMesh);

        // Every new World gets a default Main Camera
        EditorObject cam;
        cam.id = w.nextObjectId++;
        cam.name = "Main Camera";
        cam.tag = "MainCamera";
        cam.camera.enabled = true;
        cam.camera.mainCamera = true;
        cam.camera.backclear = true;
        w.objects.push_back(cam);

        project.worlds.push_back(w);
        activeWorldIndex = static_cast<int>(project.worlds.size()) - 1;
        selectedObjectId = cam.id;
        selectedUIElementId = -1;
        activeParticles.clear();
        isDirty = true;

        logInfo("Created new World '" + w.name + "' in Project '" + project.projectName + "'.");
        return activeWorldIndex;
    }

    int EditorState::duplicateWorld(int worldIdx)
    {
        if (worldIdx < 0 || worldIdx >= static_cast<int>(project.worlds.size()))
            return -1;

        EditorWorld copy = project.worlds[worldIdx];
        copy.name += "_Copy";
        project.worlds.push_back(copy);
        activeWorldIndex = static_cast<int>(project.worlds.size()) - 1;
        selectedObjectId = copy.objects.empty() ? -1 : copy.objects.front().id;
        selectedUIElementId = copy.guiElements.empty() ? -1 : copy.guiElements.front().id;
        isDirty = true;

        logInfo("Duplicated World -> '" + copy.name + "'.");
        return activeWorldIndex;
    }

    bool EditorState::deleteWorld(int worldIdx)
    {
        if (project.worlds.size() <= 1)
        {
            logWarn("Cannot delete the last remaining World in the Project.");
            return false;
        }
        if (worldIdx < 0 || worldIdx >= static_cast<int>(project.worlds.size()))
            return false;

        std::string removedName = project.worlds[worldIdx].name;
        project.worlds.erase(project.worlds.begin() + worldIdx);
        activeWorldIndex = std::clamp(activeWorldIndex, 0, static_cast<int>(project.worlds.size()) - 1);
        project.startupWorldIndex = std::clamp(project.startupWorldIndex, 0, static_cast<int>(project.worlds.size()) - 1);
        selectedObjectId = activeWorld().objects.empty() ? -1 : activeWorld().objects.front().id;
        selectedUIElementId = activeWorld().guiElements.empty() ? -1 : activeWorld().guiElements.front().id;
        activeParticles.clear();
        isDirty = true;

        logInfo("Deleted World '" + removedName + "'.");
        return true;
    }

    bool EditorState::selectWorld(int worldIdx)
    {
        if (worldIdx < 0 || worldIdx >= static_cast<int>(project.worlds.size()))
            return false;
        activeWorldIndex = worldIdx;
        const EditorWorld &w = activeWorld();
        selectedObjectId = w.objects.empty() ? -1 : w.objects.front().id;
        selectedUIElementId = w.guiElements.empty() ? -1 : w.guiElements.front().id;
        activeParticles.clear();
        updateNavMeshPreview(false, false);
        runPhysicsStormCast();
        logInfo("Switched active World to '" + w.name + "'.");
        return true;
    }

    int EditorState::findWorldIndexByName(const std::string &worldName) const
    {
        for (size_t i = 0; i < project.worlds.size(); ++i)
        {
            if (project.worlds[i].name == worldName)
                return static_cast<int>(i);
        }
        return -1;
    }

    EditorObject &EditorState::createObject(const std::string &name, int parentId)
    {
        recordUndo("Create GameObject: " + name);
        EditorWorld &w = activeWorld();
        EditorObject obj;
        obj.id = w.allocateObjectId();
        obj.name = name.empty() ? ("GameObject_" + std::to_string(obj.id)) : name;
        obj.parentId = parentId;
        w.objects.push_back(obj);
        selectedObjectId = obj.id;
        isDirty = true;
        return w.objects.back();
    }

    void EditorState::ensureComponentIds(EditorObject &obj)
    {
        EditorWorld &w = activeWorld();
        if (obj.sprite.enabled && obj.sprite.componentId == 0) obj.sprite.componentId = w.allocateComponentId();
        if (obj.camera.enabled && obj.camera.componentId == 0) obj.camera.componentId = w.allocateComponentId();
        if (obj.light.enabled && obj.light.componentId == 0) obj.light.componentId = w.allocateComponentId();
        if (obj.terrain.enabled && obj.terrain.componentId == 0) obj.terrain.componentId = w.allocateComponentId();
        if (obj.particles.enabled && obj.particles.componentId == 0) obj.particles.componentId = w.allocateComponentId();
        if (obj.physics.colliderEnabled && obj.physics.componentId == 0) obj.physics.componentId = w.allocateComponentId();
        if (obj.rigidbody2D.enabled && obj.rigidbody2D.componentId == 0) obj.rigidbody2D.componentId = w.allocateComponentId();
        if (obj.boxCollider2D.enabled && obj.boxCollider2D.componentId == 0) obj.boxCollider2D.componentId = w.allocateComponentId();
        if (obj.multiCollider2D.enabled && obj.multiCollider2D.componentId == 0) obj.multiCollider2D.componentId = w.allocateComponentId();
        if (obj.navMeshAgent.enabled && obj.navMeshAgent.componentId == 0) obj.navMeshAgent.componentId = w.allocateComponentId();
        if (obj.audio.enabled && obj.audio.componentId == 0) obj.audio.componentId = w.allocateComponentId();
        if (obj.script.enabled && obj.script.componentId == 0) obj.script.componentId = w.allocateComponentId();
    }

    EditorObject *EditorState::findObject(int id)
    {
        for (auto &obj : activeWorld().objects)
        {
            if (obj.id == id)
                return &obj;
        }
        return nullptr;
    }

    const EditorObject *EditorState::findObject(int id) const
    {
        for (const auto &obj : activeWorld().objects)
        {
            if (obj.id == id)
                return &obj;
        }
        return nullptr;
    }

    static void reassignComponentIds(EditorWorld &w, EditorObject &obj)
    {
        if (obj.sprite.enabled) obj.sprite.componentId = w.allocateComponentId();
        if (obj.camera.enabled) obj.camera.componentId = w.allocateComponentId();
        if (obj.light.enabled) obj.light.componentId = w.allocateComponentId();
        if (obj.terrain.enabled) obj.terrain.componentId = w.allocateComponentId();
        if (obj.particles.enabled) obj.particles.componentId = w.allocateComponentId();
        if (obj.physics.colliderEnabled || obj.physics.moveControllerEnabled) obj.physics.componentId = w.allocateComponentId();
        if (obj.rigidbody2D.enabled) obj.rigidbody2D.componentId = w.allocateComponentId();
        if (obj.boxCollider2D.enabled) obj.boxCollider2D.componentId = w.allocateComponentId();
        if (obj.multiCollider2D.enabled) obj.multiCollider2D.componentId = w.allocateComponentId();
        if (obj.navMeshAgent.enabled) obj.navMeshAgent.componentId = w.allocateComponentId();
        if (obj.audio.enabled) obj.audio.componentId = w.allocateComponentId();
        if (obj.script.enabled) obj.script.componentId = w.allocateComponentId();
    }

    int EditorState::duplicateObject(int id)
    {
        const EditorObject *src = findObject(id);
        if (!src)
            return -1;

        recordUndo("Duplicate GameObject: " + src->name);
        EditorWorld &w = activeWorld();
        EditorObject copy = *src;
        copy.id = w.allocateObjectId();
        copy.name = src->name + " (Copy)";
        copy.position.x += 0.5f;
        copy.position.y -= 0.5f;
        reassignComponentIds(w, copy);

        w.objects.push_back(copy);
        int newRootId = copy.id;

        // Recursively duplicate any children
        std::function<void(int, int)> duplicateChildren = [&](int oldParentId, int newParentId) {
            std::vector<int> childIds;
            for (const auto &o : w.objects)
            {
                if (o.parentId == oldParentId)
                    childIds.push_back(o.id);
            }
            for (int cId : childIds)
            {
                const EditorObject *cSrc = findObject(cId);
                if (!cSrc) continue;
                EditorObject cCopy = *cSrc;
                cCopy.id = w.allocateObjectId();
                cCopy.parentId = newParentId;
                reassignComponentIds(w, cCopy);
                w.objects.push_back(cCopy);
                duplicateChildren(cId, cCopy.id);
            }
        };
        duplicateChildren(id, newRootId);

        selectObject(newRootId, false);
        isDirty = true;
        logInfo("Duplicated GameObject '" + copy.name + "' [ID: " + std::to_string(newRootId) + "] in World '" + w.name + "'.");
        return newRootId;
    }

    bool EditorState::deleteObject(int id)
    {
        EditorWorld &w = activeWorld();
        auto it = std::find_if(w.objects.begin(), w.objects.end(), [id](const EditorObject &o) { return o.id == id; });
        if (it == w.objects.end())
            return false;

        recordUndo("Delete GameObject: " + it->name);
        std::string removedName = it->name;
        w.objects.erase(it);

        for (auto &obj : w.objects)
        {
            if (obj.parentId == id)
                obj.parentId = -1;
        }

        auto selIt = std::find(selectedObjectIds.begin(), selectedObjectIds.end(), id);
        if (selIt != selectedObjectIds.end())
        {
            selectedObjectIds.erase(selIt);
        }

        if (selectedObjectId == id)
        {
            selectedObjectId = selectedObjectIds.empty() ? (w.objects.empty() ? -1 : w.objects.front().id) : selectedObjectIds.back();
        }
        isDirty = true;
        logInfo("Deleted GameObject '" + removedName + "' from World '" + w.name + "'.");
        return true;
    }

    bool EditorState::isObjectSelected(int id) const
    {
        if (id == -1) return false;
        if (selectedObjectId == id) return true;
        return std::find(selectedObjectIds.begin(), selectedObjectIds.end(), id) != selectedObjectIds.end();
    }

    void EditorState::selectObject(int id, bool addToSelection)
    {
        inspectAssetMode = false;
        if (!addToSelection)
        {
            selectedObjectIds.clear();
            if (id != -1)
            {
                selectedObjectIds.push_back(id);
            }
            selectedObjectId = id;
        }
        else
        {
            if (id == -1) return;
            auto it = std::find(selectedObjectIds.begin(), selectedObjectIds.end(), id);
            if (it != selectedObjectIds.end())
            {
                selectedObjectIds.erase(it);
                if (selectedObjectId == id)
                {
                    selectedObjectId = selectedObjectIds.empty() ? -1 : selectedObjectIds.back();
                }
            }
            else
            {
                selectedObjectIds.push_back(id);
                selectedObjectId = id;
            }
        }
    }

    void EditorState::deselectObject(int id)
    {
        auto it = std::find(selectedObjectIds.begin(), selectedObjectIds.end(), id);
        if (it != selectedObjectIds.end())
        {
            selectedObjectIds.erase(it);
        }
        if (selectedObjectId == id)
        {
            selectedObjectId = selectedObjectIds.empty() ? -1 : selectedObjectIds.back();
        }
    }

    void EditorState::clearSelection()
    {
        selectedObjectIds.clear();
        selectedObjectId = -1;
    }

    void EditorState::deleteSelectedObjects()
    {
        auto ids = selectedObjectIds;
        if (ids.empty() && selectedObjectId != -1)
        {
            ids.push_back(selectedObjectId);
        }
        for (int id : ids)
        {
            deleteObject(id);
        }
        clearSelection();
    }

    Vec2 EditorState::getObjectWorldPosition(const EditorObject &obj) const
    {
        Vec2 pos = obj.position;
        int pid = obj.parentId;
        int guard = 0;
        while (pid != -1 && guard++ < 16)
        {
            const EditorObject *parent = findObject(pid);
            if (!parent || parent->id == obj.id)
                break;
            float rad = parent->angle * (Math::Pi / 180.0f);
            float cs = std::cos(rad);
            float sn = std::sin(rad);
            Vec2 scaled(pos.x * parent->scale.x, pos.y * parent->scale.y);
            Vec2 rotated(scaled.x * cs - scaled.y * sn, scaled.x * sn + scaled.y * cs);
            pos = parent->position + rotated;
            pid = parent->parentId;
        }
        return pos;
    }

    float EditorState::getObjectWorldAngle(const EditorObject &obj) const
    {
        float ang = obj.angle;
        int pid = obj.parentId;
        int guard = 0;
        while (pid != -1 && guard++ < 16)
        {
            const EditorObject *parent = findObject(pid);
            if (!parent || parent->id == obj.id)
                break;
            ang += parent->angle;
            pid = parent->parentId;
        }
        return ang;
    }

    Vec2 EditorState::getObjectWorldScale(const EditorObject &obj) const
    {
        Vec2 sc = obj.scale;
        int pid = obj.parentId;
        int guard = 0;
        while (pid != -1 && guard++ < 16)
        {
            const EditorObject *parent = findObject(pid);
            if (!parent || parent->id == obj.id)
                break;
            sc.x *= parent->scale.x;
            sc.y *= parent->scale.y;
            pid = parent->parentId;
        }
        return sc;
    }

    const EditorObject *EditorState::findMainCameraObject() const
    {
        const EditorObject *fallbackCam = nullptr;
        for (const auto &obj : activeWorld().objects)
        {
            if (obj.active && obj.camera.enabled)
            {
                if (obj.camera.mainCamera)
                    return &obj;
                if (!fallbackCam)
                    fallbackCam = &obj;
            }
        }
        return fallbackCam;
    }

    Rectf EditorState::getObjectWorldBounds(const EditorObject &obj) const
    {
        Vec2 wp = getObjectWorldPosition(obj);
        Vec2 ws = getObjectWorldScale(obj);
        float w = 0.7f;
        float h = 0.7f;
        Vec2 pivot(0.5f, 0.5f);

        if (obj.sprite.enabled)
        {
            w = std::max(0.2f, obj.sprite.size.x * std::fabs(ws.x));
            h = std::max(0.2f, obj.sprite.size.y * std::fabs(ws.y));
            pivot = obj.sprite.pivot;
        }
        else if (obj.terrain.enabled)
        {
            w = std::max(1.0f, obj.terrain.width * obj.terrain.cellScale);
            h = std::max(1.0f, obj.terrain.length * obj.terrain.cellScale);
        }
        else if (obj.boxCollider2D.enabled)
        {
            w = std::max(0.2f, obj.boxCollider2D.size.x * std::fabs(ws.x));
            h = std::max(0.2f, obj.boxCollider2D.size.y * std::fabs(ws.y));
        }

        return Rectf(wp.x - w * pivot.x, wp.y - h * pivot.y, w, h);
    }

    int EditorState::pickObjectAt(const Vec2 &worldPos) const
    {
        std::vector<const EditorObject *> sorted;
        for (const auto &obj : activeWorld().objects)
        {
            if (obj.active)
                sorted.push_back(&obj);
        }

        std::stable_sort(sorted.begin(), sorted.end(), [](const EditorObject *a, const EditorObject *b) {
            return a->zOrder > b->zOrder;
        });

        for (const EditorObject *obj : sorted)
        {
            Vec2 wp = getObjectWorldPosition(*obj);
            Vec2 ws = getObjectWorldScale(*obj);
            float angle = getObjectWorldAngle(*obj);

            float w = 0.7f * std::max(0.1f, std::fabs(ws.x));
            float h = 0.7f * std::max(0.1f, std::fabs(ws.y));
            Vec2 pivot(0.5f, 0.5f);

            if (obj->sprite.enabled)
            {
                w = std::max(0.2f, obj->sprite.size.x * std::fabs(ws.x));
                h = std::max(0.2f, obj->sprite.size.y * std::fabs(ws.y));
                pivot = obj->sprite.pivot;
            }
            else if (obj->terrain.enabled)
            {
                w = std::max(1.0f, obj->terrain.width * obj->terrain.cellScale);
                h = std::max(1.0f, obj->terrain.length * obj->terrain.cellScale);
            }
            else if (obj->boxCollider2D.enabled)
            {
                w = std::max(0.2f, obj->boxCollider2D.size.x * std::fabs(ws.x));
                h = std::max(0.2f, obj->boxCollider2D.size.y * std::fabs(ws.y));
            }

            // Test in local rotated space
            float dx = worldPos.x - wp.x;
            float dy = worldPos.y - wp.y;
            float rad = -angle * (Math::Pi / 180.0f);
            float cosA = std::cos(rad);
            float sinA = std::sin(rad);
            float localX = dx * cosA - dy * sinA;
            float localY = dx * sinA + dy * cosA;

            float minX = -w * pivot.x;
            float maxX = w * (1.0f - pivot.x);
            float minY = -h * pivot.y;
            float maxY = h * (1.0f - pivot.y);

            if (localX >= minX && localX <= maxX && localY >= minY && localY <= maxY)
            {
                return obj->id;
            }
        }
        return -1;
    }

    EditorObject *EditorState::findActiveTerrainObject()
    {
        if (EditorObject *sel = findObject(selectedObjectId))
        {
            if (sel->terrain.enabled)
            {
                sel->terrain.ensureGridInitialized(0);
                return sel;
            }
        }
        for (auto &obj : activeWorld().objects)
        {
            if (obj.active && obj.terrain.enabled)
            {
                obj.terrain.ensureGridInitialized(0);
                return &obj;
            }
        }
        return nullptr;
    }

    const EditorObject *EditorState::findActiveTerrainObject() const
    {
        if (const EditorObject *sel = findObject(selectedObjectId))
        {
            if (sel->terrain.enabled)
                return sel;
        }
        for (const auto &obj : activeWorld().objects)
        {
            if (obj.active && obj.terrain.enabled)
                return &obj;
        }
        return nullptr;
    }

    Color EditorState::getTerrainMaterialBaseColor(int materialId, const Color &customFallback)
    {
        switch (materialId)
        {
            case -1: return Color(0, 0, 0, 0);              // Empty / Hole
            case 0:  return Color(54, 128, 68, 240);        // Grass
            case 1:  return Color(124, 92, 58, 240);        // Dirt / Soil
            case 2:  return Color(112, 118, 128, 245);      // Stone / Rock
            case 3:  return Color(214, 188, 118, 240);      // Sand / Desert
            case 4:  return Color(36, 114, 188, 235);       // Water
            case 5:  return Color(96, 102, 114, 245);       // Cobblestone / Road
            case 6:  return Color(224, 238, 250, 245);      // Snow / Ice
            case 7:  return Color(228, 72, 28, 245);        // Lava / Magma
            case 8:  return Color(168, 132, 78, 245);       // Wood / Tatami
            case 9:  return customFallback;                 // Custom Color / Texture
            default: return customFallback;
        }
    }

    const char *EditorState::getTerrainMaterialName(int materialId)
    {
        switch (materialId)
        {
            case -1: return "Empty (Erased)";
            case 0:  return "Grass (Трава)";
            case 1:  return "Dirt / Soil (Земля)";
            case 2:  return "Stone / Rock (Камень)";
            case 3:  return "Sand (Песок)";
            case 4:  return "Water (Вода)";
            case 5:  return "Cobblestone (Брусчатка)";
            case 6:  return "Snow / Ice (Снег)";
            case 7:  return "Lava / Magma (Лава)";
            case 8:  return "Wood / Tatami (Татами)";
            case 9:  return "Custom Texture / Color";
            default: return "Unknown";
        }
    }

    void EditorState::fillTerrainAll(Terrain2DData &terrain, int materialId, const Color &color)
    {
        terrain.ensureGridInitialized(materialId >= -1 ? materialId : 0);
        Color fillCol = (materialId == 9) ? color : getTerrainMaterialBaseColor(materialId, color);
        std::fill(terrain.cellMaterials.begin(), terrain.cellMaterials.end(), materialId);
        std::fill(terrain.cellColors.begin(), terrain.cellColors.end(), fillCol);
        isDirty = true;
    }

    void EditorState::floodFillTerrainCell(Terrain2DData &terrain, int startCol, int startRow, int newMaterial, const Color &newColor)
    {
        terrain.ensureGridInitialized(0);
        int w = terrain.width;
        int l = terrain.length;
        if (startCol < 0 || startCol >= w || startRow < 0 || startRow >= l)
            return;

        int targetMat = terrain.cellMaterials[startRow * w + startCol];
        Color fillCol = (newMaterial == 9) ? newColor : getTerrainMaterialBaseColor(newMaterial, newColor);
        if (targetMat == newMaterial && terrain.cellColors[startRow * w + startCol] == fillCol)
            return;

        std::vector<Vec2Int> stack;
        stack.push_back(Vec2Int(startCol, startRow));
        std::vector<bool> visited(static_cast<size_t>(w * l), false);

        while (!stack.empty())
        {
            Vec2Int cur = stack.back();
            stack.pop_back();
            if (cur.x < 0 || cur.x >= w || cur.y < 0 || cur.y >= l)
                continue;
            size_t idx = static_cast<size_t>(cur.y * w + cur.x);
            if (visited[idx] || terrain.cellMaterials[idx] != targetMat)
                continue;

            visited[idx] = true;
            terrain.cellMaterials[idx] = newMaterial;
            terrain.cellColors[idx] = fillCol;

            stack.push_back(Vec2Int(cur.x + 1, cur.y));
            stack.push_back(Vec2Int(cur.x - 1, cur.y));
            stack.push_back(Vec2Int(cur.x, cur.y + 1));
            stack.push_back(Vec2Int(cur.x, cur.y - 1));
        }
        isDirty = true;
    }

    void EditorState::applyTerrainBorderColliders(Terrain2DData &terrain, bool state)
    {
        terrain.ensureGridInitialized(0);
        int w = terrain.width;
        int l = terrain.length;
        for (int r = 0; r < l; ++r)
        {
            for (int c = 0; c < w; ++c)
            {
                if (r == 0 || r == l - 1 || c == 0 || c == w - 1)
                    terrain.cellColliders[r * w + c] = state;
            }
        }
        isDirty = true;
        updateNavMeshPreview(false, false);
    }

    void EditorState::generateProceduralTerrain(Terrain2DData &terrain, int preset)
    {
        int w = std::max(4, terrain.width);
        int l = std::max(4, terrain.length);
        terrain.width = w;
        terrain.length = l;
        size_t total = static_cast<size_t>(w * l);
        terrain.cellMaterials.assign(total, 0);
        terrain.cellColors.assign(total, getTerrainMaterialBaseColor(0, terrain.tileColor));
        terrain.cellColliders.assign(total, false);
        terrain.paintStrokes.clear();

        if (preset == 0) // Samurai Dojo Courtyard (Grass, Cobble Path, Tatami Platform, Water Pond)
        {
            int midR = l / 2;
            int midC = w / 2;
            for (int r = 0; r < l; ++r)
            {
                for (int c = 0; c < w; ++c)
                {
                    size_t idx = static_cast<size_t>(r * w + c);
                    int mat = 0; // Grass
                    if (r == 0 || r == l - 1 || c == 0 || c == w - 1)
                    {
                        mat = 2; // Stone border wall
                        if (terrain.generateBorderColliders)
                            terrain.cellColliders[idx] = true;
                    }
                    else if (r == midR || c == midC)
                    {
                        mat = 5; // Cobblestone cross path
                    }
                    else if (c >= 2 && c <= 4 && r >= 2 && r <= 4)
                    {
                        mat = 8; // Wood / Tatami platform near Torii
                    }
                    else if (c >= w - 5 && c <= w - 3 && r >= l - 4 && r <= l - 2)
                    {
                        mat = 4; // Water koi pond
                        terrain.cellColliders[idx] = true;
                    }
                    else if ((r + c) % 7 == 0)
                    {
                        mat = 1; // Dirt patches
                    }
                    terrain.cellMaterials[idx] = mat;
                    terrain.cellColors[idx] = getTerrainMaterialBaseColor(mat, terrain.tileColor);
                }
            }
            // Add soft surface brush strokes for natural blending
            TerrainPaintStroke s1;
            s1.uvPos = Vec2(0.32f, 0.35f);
            s1.radiusNormalized = 0.14f;
            s1.hardness = 0.35f;
            s1.materialId = 3; // Sand/gravel around shrine
            s1.color = Color(214, 188, 118, 165);
            terrain.paintStrokes.push_back(s1);

            TerrainPaintStroke s2;
            s2.uvPos = Vec2(0.72f, 0.70f);
            s2.radiusNormalized = 0.12f;
            s2.hardness = 0.40f;
            s2.materialId = 4; // Water glow
            s2.color = Color(48, 152, 224, 175);
            terrain.paintStrokes.push_back(s2);
        }
        else if (preset == 1) // Dungeon Obsidian & Lava Arena
        {
            for (int r = 0; r < l; ++r)
            {
                for (int c = 0; c < w; ++c)
                {
                    size_t idx = static_cast<size_t>(r * w + c);
                    int mat = 2; // Dark Stone
                    if (r == 0 || r == l - 1 || c == 0 || c == w - 1)
                    {
                        mat = 7; // Lava moat border
                        terrain.cellColliders[idx] = true;
                    }
                    else if (r == l / 2 || c == w / 2)
                    {
                        mat = 5; // Cobblestone bridge
                    }
                    else if ((c == 3 || c == w - 4) && (r == 2 || r == l - 3))
                    {
                        mat = 7; // Lava pits
                        terrain.cellColliders[idx] = true;
                    }
                    terrain.cellMaterials[idx] = mat;
                    Color base = getTerrainMaterialBaseColor(mat, terrain.tileColor);
                    if (mat == 2)
                        base = Color(64, 56, 78, 245);
                    terrain.cellColors[idx] = base;
                }
            }
            TerrainPaintStroke lavaGlow;
            lavaGlow.uvPos = Vec2(0.5f, 0.5f);
            lavaGlow.radiusNormalized = 0.18f;
            lavaGlow.hardness = 0.25f;
            lavaGlow.materialId = 7;
            lavaGlow.color = Color(245, 85, 35, 160);
            terrain.paintStrokes.push_back(lavaGlow);
        }
        else // Snow / Mountain Pass
        {
            for (int r = 0; r < l; ++r)
            {
                for (int c = 0; c < w; ++c)
                {
                    size_t idx = static_cast<size_t>(r * w + c);
                    int mat = 6; // Snow
                    if (r == 0 || r == l - 1 || c == 0 || c == w - 1)
                    {
                        mat = 2; // Rock cliff
                        terrain.cellColliders[idx] = true;
                    }
                    else if (std::abs(r - l / 2) <= 1)
                    {
                        mat = 5; // Frozen road
                    }
                    terrain.cellMaterials[idx] = mat;
                    terrain.cellColors[idx] = getTerrainMaterialBaseColor(mat, terrain.tileColor);
                }
            }
        }
        isDirty = true;
        updateNavMeshPreview(false, false);
    }

    bool EditorState::paintTerrainAtUV(Terrain2DData &terrain, const Vec2 &uvPos, bool isDragContinue)
    {
        if (uvPos.x < 0.0f || uvPos.x > 1.0f || uvPos.y < 0.0f || uvPos.y > 1.0f)
            return false;

        terrain.ensureGridInitialized(0);
        int w = terrain.width;
        int l = terrain.length;
        int col = std::clamp(static_cast<int>(uvPos.x * w), 0, w - 1);
        int row = std::clamp(static_cast<int>(uvPos.y * l), 0, l - 1);

        int brushCells = std::max(0, static_cast<int>(std::floor(terrainBrushRadius - 0.35f)));
        Color activeCol = (terrainActiveMaterial == 9)
                              ? terrainBrushColor
                              : getTerrainMaterialBaseColor(terrainActiveMaterial, terrainBrushColor);
        activeCol.a = static_cast<uint8_t>(std::clamp(std::round(terrainBrushOpacity * 255.0f), 10.0f, 255.0f));

        switch (terrainBrushTool)
        {
            case TerrainBrushTool::PaintTileMaterial:
            {
                for (int r = std::max(0, row - brushCells); r <= std::min(l - 1, row + brushCells); ++r)
                {
                    for (int c = std::max(0, col - brushCells); c <= std::min(w - 1, col + brushCells); ++c)
                    {
                        float dx = static_cast<float>(c - col);
                        float dy = static_cast<float>(r - row);
                        if (brushCells == 0 || (dx * dx + dy * dy) <= (terrainBrushRadius * terrainBrushRadius))
                        {
                            size_t idx = static_cast<size_t>(r * w + c);
                            terrain.cellMaterials[idx] = terrainActiveMaterial;
                            terrain.cellColors[idx] = activeCol;
                            if (terrainActiveMaterial == 4 || terrainActiveMaterial == 7)
                                terrain.cellColliders[idx] = true;
                        }
                    }
                }
                break;
            }
            case TerrainBrushTool::FreehandSurfaceBrush:
            {
                float normRad = std::clamp(terrainBrushRadius / static_cast<float>(std::max(1, std::min(w, l))), 0.02f, 0.45f);
                if (isDragContinue && !terrain.paintStrokes.empty())
                {
                    const auto &last = terrain.paintStrokes.back();
                    float d = Vec2::Distance(last.uvPos, uvPos);
                    if (d < normRad * 0.22f)
                        return false;
                }
                TerrainPaintStroke stroke;
                stroke.uvPos = uvPos;
                stroke.radiusNormalized = normRad;
                stroke.radiusCells = terrainBrushRadius;
                stroke.hardness = std::clamp(terrainBrushHardness, 0.0f, 1.0f);
                stroke.opacity = std::clamp(terrainBrushOpacity, 0.05f, 1.0f);
                stroke.materialId = terrainActiveMaterial;
                stroke.color = activeCol;
                terrain.paintStrokes.push_back(stroke);
                if (terrain.paintStrokes.size() > 512)
                    terrain.paintStrokes.erase(terrain.paintStrokes.begin());
                break;
            }
            case TerrainBrushTool::BucketFill:
            {
                if (!isDragContinue)
                    floodFillTerrainCell(terrain, col, row, terrainActiveMaterial, activeCol);
                break;
            }
            case TerrainBrushTool::RectFill:
            {
                int rx = std::max(1, brushCells + 1);
                for (int r = std::max(0, row - rx); r <= std::min(l - 1, row + rx); ++r)
                {
                    for (int c = std::max(0, col - rx); c <= std::min(w - 1, col + rx); ++c)
                    {
                        size_t idx = static_cast<size_t>(r * w + c);
                        terrain.cellMaterials[idx] = terrainActiveMaterial;
                        terrain.cellColors[idx] = activeCol;
                    }
                }
                break;
            }
            case TerrainBrushTool::PaintCollider:
            case TerrainBrushTool::EraseCollider:
            {
                bool lockVal = (terrainBrushTool == TerrainBrushTool::PaintCollider);
                for (int r = std::max(0, row - brushCells); r <= std::min(l - 1, row + brushCells); ++r)
                {
                    for (int c = std::max(0, col - brushCells); c <= std::min(w - 1, col + brushCells); ++c)
                    {
                        terrain.cellColliders[r * w + c] = lockVal;
                    }
                }
                break;
            }
            case TerrainBrushTool::Eraser:
            {
                float normRad = std::clamp(terrainBrushRadius / static_cast<float>(std::max(1, std::min(w, l))), 0.03f, 0.45f);
                terrain.paintStrokes.erase(
                    std::remove_if(
                        terrain.paintStrokes.begin(),
                        terrain.paintStrokes.end(),
                        [&](const TerrainPaintStroke &s) {
                            return Vec2::Distance(s.uvPos, uvPos) <= (normRad + s.radiusNormalized * 0.6f);
                        }),
                    terrain.paintStrokes.end());

                for (int r = std::max(0, row - brushCells); r <= std::min(l - 1, row + brushCells); ++r)
                {
                    for (int c = std::max(0, col - brushCells); c <= std::min(w - 1, col + brushCells); ++c)
                    {
                        size_t idx = static_cast<size_t>(r * w + c);
                        terrain.cellMaterials[idx] = -1;
                        terrain.cellColors[idx] = Color(0, 0, 0, 0);
                        terrain.cellColliders[idx] = false;
                    }
                }
                break;
            }
        }

        isDirty = true;
        if (terrainBrushTool == TerrainBrushTool::PaintCollider ||
            terrainBrushTool == TerrainBrushTool::EraseCollider ||
            terrainBrushTool == TerrainBrushTool::Eraser)
        {
            updateNavMeshPreview(false, false);
        }
        return true;
    }

    EditorUIElement &EditorState::createUIElement(const std::string &name, int type)
    {
        EditorWorld &w = activeWorld();
        EditorUIElement el;
        el.id = w.nextUIElementId++;
        el.name = name.empty() ? ("UIWidget_" + std::to_string(el.id)) : name;
        el.type = type;
        el.rect = Rect(24, 24 + static_cast<int>(w.guiElements.size()) * 38, 220, 32);
        el.text = el.name;
        w.guiElements.push_back(el);
        selectedUIElementId = el.id;
        isDirty = true;
        return w.guiElements.back();
    }

    EditorUIElement *EditorState::findUIElement(int id)
    {
        for (auto &el : activeWorld().guiElements)
        {
            if (el.id == id)
                return &el;
        }
        return nullptr;
    }

    const EditorUIElement *EditorState::findUIElement(int id) const
    {
        for (const auto &el : activeWorld().guiElements)
        {
            if (el.id == id)
                return &el;
        }
        return nullptr;
    }

    bool EditorState::deleteUIElement(int id)
    {
        EditorWorld &w = activeWorld();
        auto it = std::find_if(w.guiElements.begin(), w.guiElements.end(), [id](const EditorUIElement &e) { return e.id == id; });
        if (it == w.guiElements.end())
            return false;
        w.guiElements.erase(it);
        selectedUIElementId = w.guiElements.empty() ? -1 : w.guiElements.front().id;
        isDirty = true;
        return true;
    }

    int EditorState::pickUIElementAt(int screenX, int screenY, int viewW, int viewH) const
    {
        if (viewW <= 0 || viewH <= 0)
            return -1;
        float scaleX = static_cast<float>(project.resolutionWidth) / static_cast<float>(viewW);
        float scaleY = static_cast<float>(project.resolutionHeight) / static_cast<float>(viewH);
        int gx = static_cast<int>(screenX * scaleX);
        int gy = static_cast<int>(screenY * scaleY);

        const auto &els = activeWorld().guiElements;
        for (auto it = els.rbegin(); it != els.rend(); ++it)
        {
            if (!it->visible)
                continue;
            if (gx >= it->rect.x && gx <= it->rect.x + it->rect.w &&
                gy >= it->rect.y && gy <= it->rect.y + it->rect.h)
            {
                return it->id;
            }
        }
        return -1;
    }

    void EditorState::setPlayState(EditorPlayState newState)
    {
        if (playState == newState)
            return;

        if (playState == EditorPlayState::Stopped && newState == EditorPlayState::Playing)
        {
            savedPlayProjectSnapshot = project;
            savedActiveWorldIndex = activeWorldIndex;
            simTime = 0.0f;
            simFrames = 0;
            emitAccumulator = 0.0f;
            activeParticles.clear();
            logInfo("Entered Play Mode in World '" + activeWorld().name + "' (Project snapshot saved).");
        }
        else if (newState == EditorPlayState::Stopped)
        {
            if (!savedPlayProjectSnapshot.worlds.empty())
            {
                project = savedPlayProjectSnapshot;
                activeWorldIndex = std::clamp(savedActiveWorldIndex, 0, static_cast<int>(project.worlds.size()) - 1);
                savedPlayProjectSnapshot.worlds.clear();
            }
            simTime = 0.0f;
            simFrames = 0;
            activeParticles.clear();
            logInfo("Stopped Play Mode (Project & Worlds restored).");
        }
        else if (newState == EditorPlayState::Paused)
        {
            logInfo("Simulation paused at t=" + std::to_string(simTime) + "s.");
        }

        playState = newState;
    }

    void EditorState::stepSimulation(float dt)
    {
        if (dt <= 0.0f)
            return;

        simTime += dt;
        simFrames++;
        emitAccumulator += dt;

        EditorWorld &w = activeWorld();

        // 1. Update GameObjects with MotionScript & ParticleSystem
        for (auto &obj : w.objects)
        {
            if (!obj.active)
                continue;

            if (obj.script.enabled)
            {
                if (obj.script.angularVelocity != 0.0f)
                {
                    obj.angle += obj.script.angularVelocity * dt;
                    if (obj.angle > 180.0f)
                        obj.angle -= 360.0f;
                    if (obj.angle < -180.0f)
                        obj.angle += 360.0f;
                }

                if (obj.script.pulseSpeed != 0.0f && obj.script.pulseAmplitude != 0.0f)
                {
                    float wave = Math::Sin(simTime * obj.script.pulseSpeed);
                    if (obj.sprite.enabled)
                        obj.position.y += wave * obj.script.pulseAmplitude * dt;
                    if (obj.light.enabled)
                        obj.light.intensity = std::max(0.2f, 1.0f + wave * obj.script.pulseAmplitude);
                }

                if (obj.script.linearVelocity.x != 0.0f || obj.script.linearVelocity.y != 0.0f)
                {
                    float dir = Math::Cos(simTime * 1.2f);
                    obj.position.x += obj.script.linearVelocity.x * dir * dt;
                    obj.position.y += obj.script.linearVelocity.y * dir * dt;
                }
            }

            // Emit particles for active ParticleSystem components
            if (obj.particles.enabled && obj.particles.emit)
            {
                int currentForObj = 0;
                for (const auto &p : activeParticles)
                {
                    if (p.ownerObjectId == obj.id)
                        ++currentForObj;
                }

                if (obj.particles.maxLimit <= 0 || currentForObj < obj.particles.maxLimit)
                {
                    SimulatedParticle sp;
                    sp.ownerObjectId = obj.id;
                    sp.worldPos = getObjectWorldPosition(obj);

                    float spread = obj.particles.randomDirection ? ((simFrames * 37 + obj.id * 19) % 100 - 50) * 0.025f : 0.0f;
                    Vec2 dir = obj.particles.direction;
                    dir.x += spread;
                    sp.velocity = dir * obj.particles.speed;
                    sp.angle = static_cast<float>((simFrames * 15) % 360);
                    sp.angularVel = obj.particles.rotate ? obj.particles.rotatePerFrame * 4.0f : 0.0f;
                    sp.age = 0.0f;
                    sp.lifetime = std::max(0.2f, obj.particles.duration);
                    sp.primitiveShape = obj.particles.primitiveSource;
                    sp.startSize = obj.particles.startSize;
                    sp.endSize = obj.particles.endSize;
                    sp.startColor = obj.particles.startColor;
                    sp.endColor = obj.particles.endColor;
                    activeParticles.push_back(sp);
                }
            }
        }

        // 2. Advance active particles
        for (auto it = activeParticles.begin(); it != activeParticles.end();)
        {
            it->age += dt;
            if (it->age >= it->lifetime)
            {
                it = activeParticles.erase(it);
            }
            else
            {
                it->worldPos.x += it->velocity.x * dt;
                it->worldPos.y += it->velocity.y * dt;
                it->angle += it->angularVel * dt;
                ++it;
            }
        }

        // 3. Live Physics 2D RigidBody simulation (Box2D)
        bool hasPhysicsObjects = false;
        for (const auto &obj : w.objects)
        {
            if (obj.active && (obj.rigidbody2D.enabled || obj.boxCollider2D.enabled || obj.multiCollider2D.enabled))
            {
                hasPhysicsObjects = true;
                break;
            }
        }

        if (hasPhysicsObjects)
        {
            box2d::World physWorld(box2d::Vec2(0.0f, -9.8f), 10);
            std::vector<std::pair<EditorObject*, std::unique_ptr<box2d::Body>>> activeBodies;

            for (auto &obj : w.objects)
            {
                if (!obj.active)
                    continue;

                if (obj.rigidbody2D.enabled || obj.boxCollider2D.enabled || obj.multiCollider2D.enabled)
                {
                    Vec2 size = {1.0f, 1.0f};
                    if (obj.boxCollider2D.enabled)
                        size = obj.boxCollider2D.size;
                    else if (obj.multiCollider2D.enabled && !obj.multiCollider2D.points.empty())
                    {
                        float minX = obj.multiCollider2D.points[0].x, maxX = minX;
                        float minY = obj.multiCollider2D.points[0].y, maxY = minY;
                        for (const auto &p : obj.multiCollider2D.points)
                        {
                            minX = std::min(minX, p.x);
                            maxX = std::max(maxX, p.x);
                            minY = std::min(minY, p.y);
                            maxY = std::max(maxY, p.y);
                        }
                        size = {std::max(0.1f, maxX - minX), std::max(0.1f, maxY - minY)};
                    }
                    else if (obj.sprite.enabled)
                        size = {obj.sprite.size.x * obj.scale.x, obj.sprite.size.y * obj.scale.y};

                    auto body = std::make_unique<box2d::Body>();
                    body->position = box2d::Vec2(obj.position.x, obj.position.y);
                    body->rotation = obj.angle * (box2d::k_pi / 180.0f);
                    body->width = box2d::Vec2(size.x, size.y);

                    if (obj.rigidbody2D.enabled)
                    {
                        body->SetBodyType(static_cast<box2d::BodyType>(obj.rigidbody2D.bodyType));
                        body->mass = std::max(0.001f, obj.rigidbody2D.mass);
                        body->linearDamping = obj.rigidbody2D.linearDamping;
                        body->angularDamping = obj.rigidbody2D.angularDamping;
                        body->gravityScale = obj.rigidbody2D.gravityScale;
                        body->freezeRotation = obj.rigidbody2D.freezeRotation;
                        body->velocity = box2d::Vec2(obj.rigidbody2D.velocity.x, obj.rigidbody2D.velocity.y);
                        body->angularVelocity = obj.rigidbody2D.angularVelocity * (box2d::k_pi / 180.0f);
                    }
                    else
                    {
                        body->SetBodyType(box2d::BodyType::Static);
                    }

                    if (obj.boxCollider2D.enabled)
                    {
                        body->friction = obj.boxCollider2D.friction;
                        body->restitution = obj.boxCollider2D.bounciness;
                        body->isTrigger = obj.boxCollider2D.isTrigger;
                    }
                    else if (obj.multiCollider2D.enabled)
                    {
                        body->friction = obj.multiCollider2D.friction;
                        body->restitution = obj.multiCollider2D.bounciness;
                        body->isTrigger = obj.multiCollider2D.isTrigger;
                    }

                    body->Set(body->width, body->mass);
                    physWorld.Add(body.get());
                    activeBodies.push_back({&obj, std::move(body)});
                }
            }

            // Step the 2D physics world
            physWorld.Step(dt);

            // Read back simulated positions and velocities
            for (auto &pair : activeBodies)
            {
                EditorObject *obj = pair.first;
                box2d::Body *body = pair.second.get();

                if (obj->rigidbody2D.enabled && obj->rigidbody2D.bodyType != 2) // Non-static
                {
                    obj->position.x = body->position.x;
                    obj->position.y = body->position.y;
                    if (!obj->rigidbody2D.freezeRotation)
                    {
                        obj->angle = body->rotation * (180.0f / box2d::k_pi);
                    }
                    obj->rigidbody2D.velocity = Vec2(body->velocity.x, body->velocity.y);
                    obj->rigidbody2D.angularVelocity = body->angularVelocity * (180.0f / box2d::k_pi);
                }
            }
        }
    }

    void EditorState::runPhysicsStormCast()
    {
        stormCastHits.clear();
        for (const auto &obj : activeWorld().objects)
        {
            if (!obj.active || !obj.physics.colliderEnabled)
                continue;
            Vec2 wp = getObjectWorldPosition(obj);
            float dx = wp.x - stormCastOrigin.x;
            float dy = wp.y - stormCastOrigin.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            float objRadius = (obj.physics.colliderType == 1)
                                  ? obj.physics.colliderRadius
                                  : std::max(obj.physics.colliderSize.x, obj.physics.colliderSize.y) * 0.5f;
            if (dist <= stormCastRadius + objRadius)
            {
                stormCastHits.push_back(obj.id);
            }
        }
    }

    static float jsonToFloat(const json &node, float fallback = 0.0f)
    {
        if (node.is_number())
            return node.get<float>();
        if (node.is_string())
        {
            try { return std::stof(node.get<std::string>()); } catch (...) {}
        }
        return fallback;
    }

    static int jsonToInt(const json &node, int fallback = 0)
    {
        if (node.is_number_integer())
            return node.get<int>();
        if (node.is_number())
            return static_cast<int>(std::round(node.get<double>()));
        if (node.is_string())
        {
            try { return std::stoi(node.get<std::string>()); } catch (...) {}
        }
        return fallback;
    }

    static uint32_t jsonToUint32(const json &node, uint32_t fallback = 0)
    {
        if (node.is_number_unsigned())
            return node.get<uint32_t>();
        if (node.is_number_integer())
            return static_cast<uint32_t>(node.get<int64_t>());
        if (node.is_number())
            return static_cast<uint32_t>(std::round(node.get<double>()));
        if (node.is_string())
        {
            try {
                return static_cast<uint32_t>(std::stoul(node.get<std::string>()));
            } catch (...) {}
        }
        return fallback;
    }

    static bool jsonToBool(const json &node, bool fallback = false)
    {
        if (node.is_boolean())
            return node.get<bool>();
        if (node.is_number_integer())
            return node.get<int64_t>() != 0;
        return fallback;
    }

    static json colorToJson(const Color &c)
    {
        json j = json::object();
        j["r"] = static_cast<int>(c.r);
        j["g"] = static_cast<int>(c.g);
        j["b"] = static_cast<int>(c.b);
        j["a"] = static_cast<int>(c.a);
        return j;
    }

    static Color jsonToColor(const json &j, const Color &fallback = Color::white)
    {
        if (!j.is_object())
            return fallback;
        int r = j.contains("r") ? jsonToInt(j.at("r"), fallback.r) : fallback.r;
        int g = j.contains("g") ? jsonToInt(j.at("g"), fallback.g) : fallback.g;
        int b = j.contains("b") ? jsonToInt(j.at("b"), fallback.b) : fallback.b;
        int a = j.contains("a") ? jsonToInt(j.at("a"), fallback.a) : fallback.a;
        return Color(r, g, b, a);
    }

    static json vec2ToJson(const Vec2 &v)
    {
        json j = json::object();
        j["x"] = static_cast<double>(v.x);
        j["y"] = static_cast<double>(v.y);
        return j;
    }

    static Vec2 jsonToVec2(const json &j, const Vec2 &fallback = Vec2::zero)
    {
        if (!j.is_object())
            return fallback;
        float x = j.contains("x") ? jsonToFloat(j.at("x"), fallback.x) : fallback.x;
        float y = j.contains("y") ? jsonToFloat(j.at("y"), fallback.y) : fallback.y;
        return Vec2(x, y);
    }

    static json serializeTerrainToJson(const Terrain2DData &ter)
    {
        json jter = json::object();
        jter["componentId"] = ter.componentId;
        jter["enabled"] = ter.enabled;
        jter["width"] = ter.width;
        jter["length"] = ter.length;
        jter["cellScale"] = static_cast<double>(ter.cellScale);
        jter["tileColor"] = colorToJson(ter.tileColor);
        jter["generateBorderColliders"] = ter.generateBorderColliders;
        jter["customTexturePath"] = ter.customTexturePath;

        json jmats = json::array();
        for (int m : ter.cellMaterials)
            jmats.push_back(m);
        jter["cellMaterials"] = jmats;

        json jcols = json::array();
        for (const auto &cc : ter.cellColors)
            jcols.push_back(colorToJson(cc));
        jter["cellColors"] = jcols;

        json jcoll = json::array();
        for (bool b : ter.cellColliders)
            jcoll.push_back(b ? 1 : 0);
        jter["cellColliders"] = jcoll;

        json jstrokes = json::array();
        for (const auto &st : ter.paintStrokes)
        {
            json js = json::object();
            js["uvPos"] = vec2ToJson(st.uvPos);
            js["radiusNormalized"] = static_cast<double>(st.radiusNormalized);
            js["radiusCells"] = static_cast<double>(st.radiusCells);
            js["hardness"] = static_cast<double>(st.hardness);
            js["opacity"] = static_cast<double>(st.opacity);
            js["materialId"] = st.materialId;
            js["color"] = colorToJson(st.color);
            jstrokes.push_back(js);
        }
        jter["paintStrokes"] = jstrokes;
        return jter;
    }

    static void deserializeTerrainFromJson(const json &jt, Terrain2DData &ter)
    {
        if (jt.contains("componentId"))
            ter.componentId = jsonToInt(jt.at("componentId"), ter.componentId);
        if (jt.contains("enabled"))
            ter.enabled = jsonToBool(jt.at("enabled"), true);
        if (jt.contains("width"))
            ter.width = jsonToInt(jt.at("width"), ter.width);
        if (jt.contains("length"))
            ter.length = jsonToInt(jt.at("length"), ter.length);
        if (jt.contains("cellScale"))
            ter.cellScale = jsonToFloat(jt.at("cellScale"), ter.cellScale);
        if (jt.contains("tileColor"))
            ter.tileColor = jsonToColor(jt.at("tileColor"), ter.tileColor);
        if (jt.contains("generateBorderColliders"))
            ter.generateBorderColliders = jsonToBool(jt.at("generateBorderColliders"), true);
        if (jt.contains("customTexturePath") && jt.at("customTexturePath").is_string())
            ter.customTexturePath = jt.at("customTexturePath").get<std::string>();

        if (jt.contains("cellMaterials") && jt.at("cellMaterials").is_array())
        {
            const auto &arr = jt.at("cellMaterials");
            ter.cellMaterials.clear();
            ter.cellMaterials.reserve(arr.size());
            for (const auto &jm : arr)
                ter.cellMaterials.push_back(jsonToInt(jm, 0));
        }
        if (jt.contains("cellColors") && jt.at("cellColors").is_array())
        {
            const auto &arr = jt.at("cellColors");
            ter.cellColors.clear();
            ter.cellColors.reserve(arr.size());
            for (const auto &jc : arr)
                ter.cellColors.push_back(jsonToColor(jc, ter.tileColor));
        }
        if (jt.contains("cellColliders") && jt.at("cellColliders").is_array())
        {
            const auto &arr = jt.at("cellColliders");
            ter.cellColliders.clear();
            ter.cellColliders.reserve(arr.size());
            for (const auto &jb : arr)
                ter.cellColliders.push_back(jsonToBool(jb, false));
        }
        if (jt.contains("paintStrokes") && jt.at("paintStrokes").is_array())
        {
            const auto &arr = jt.at("paintStrokes");
            ter.paintStrokes.clear();
            ter.paintStrokes.reserve(arr.size());
            for (const auto &jst : arr)
            {
                if (!jst.is_object())
                    continue;
                TerrainPaintStroke st;
                if (jst.contains("uvPos"))
                    st.uvPos = jsonToVec2(jst.at("uvPos"), st.uvPos);
                if (jst.contains("radiusNormalized"))
                    st.radiusNormalized = jsonToFloat(jst.at("radiusNormalized"), 0.08f);
                if (jst.contains("radiusCells"))
                    st.radiusCells = jsonToFloat(jst.at("radiusCells"), 1.0f);
                if (jst.contains("hardness"))
                    st.hardness = jsonToFloat(jst.at("hardness"), 0.65f);
                if (jst.contains("opacity"))
                    st.opacity = jsonToFloat(jst.at("opacity"), 1.0f);
                if (jst.contains("materialId"))
                    st.materialId = jsonToInt(jst.at("materialId"), 0);
                if (jst.contains("color"))
                    st.color = jsonToColor(jst.at("color"), st.color);
                ter.paintStrokes.push_back(st);
            }
        }
    }

    static json serializeObjectToJson(const EditorObject &obj)
    {
        json jo = json::object();
        jo["id"] = obj.id;
        jo["name"] = obj.name;
        jo["tag"] = obj.tag;
        jo["layer"] = obj.layer;
        jo["active"] = obj.active;
        jo["parentId"] = obj.parentId;
        jo["position"] = vec2ToJson(obj.position);
        jo["angle"] = static_cast<double>(obj.angle);
        jo["scale"] = vec2ToJson(obj.scale);
        jo["zOrder"] = obj.zOrder;

        // SpriteRenderer
        json jspr = json::object();
        jspr["componentId"] = obj.sprite.componentId;
        jspr["assetId"] = static_cast<long>(obj.sprite.assetId);
        jspr["enabled"] = obj.sprite.enabled;
        jspr["primitiveShape"] = obj.sprite.primitiveShape;
        jspr["size"] = vec2ToJson(obj.sprite.size);
        jspr["pivot"] = vec2ToJson(obj.sprite.pivot);
        jspr["color"] = colorToJson(obj.sprite.color);
        jspr["renderType"] = obj.sprite.renderType;
        jspr["renderOut"] = obj.sprite.renderOut;
        jspr["presentMode"] = obj.sprite.presentMode;
        jspr["flip"] = obj.sprite.flip;
        jspr["spritePath"] = obj.sprite.spritePath;
        jo["spriteRenderer"] = jspr;

        // Camera2D
        json jcam = json::object();
        jcam["componentId"] = obj.camera.componentId;
        jcam["enabled"] = obj.camera.enabled;
        jcam["mainCamera"] = obj.camera.mainCamera;
        jcam["backclear"] = obj.camera.backclear;
        jcam["backcolor"] = colorToJson(obj.camera.backcolor);
        jcam["zoom"] = static_cast<double>(obj.camera.zoom);
        jcam["scale"] = vec2ToJson(obj.camera.scale);
        jcam["zoomOut"] = static_cast<double>(obj.camera.zoomOut);
        jcam["distanceEvcall"] = obj.camera.distanceEvcall;
        jcam["visibleBorders"] = obj.camera.visibleBorders;
        jcam["visibleGrids"] = obj.camera.visibleGrids;
        jcam["visibleObjects"] = obj.camera.visibleObjects;
        jcam["visibleNames"] = obj.camera.visibleNames;
        jcam["cullResources"] = obj.camera.cullResources;
        jo["camera2D"] = jcam;

        // Light2D
        json jlgt = json::object();
        jlgt["componentId"] = obj.light.componentId;
        jlgt["enabled"] = obj.light.enabled;
        jlgt["lightType"] = obj.light.lightType;
        jlgt["intensity"] = static_cast<double>(obj.light.intensity);
        jlgt["range"] = static_cast<double>(obj.light.range);
        jlgt["angleDegrees"] = static_cast<double>(obj.light.angleDegrees);
        jlgt["color"] = colorToJson(obj.light.color);
        jo["light2D"] = jlgt;

        // Terrain2D
        jo["terrain2D"] = serializeTerrainToJson(obj.terrain);

        // ParticleSystem
        json jprt = json::object();
        jprt["componentId"] = obj.particles.componentId;
        jprt["enabled"] = obj.particles.enabled;
        jprt["emit"] = obj.particles.emit;
        jprt["loop"] = obj.particles.loop;
        jprt["rotate"] = obj.particles.rotate;
        jprt["worldSpace"] = obj.particles.worldSpace;
        jprt["randomDirection"] = obj.particles.randomDirection;
        jprt["interval"] = static_cast<double>(obj.particles.interval);
        jprt["speed"] = static_cast<double>(obj.particles.speed);
        jprt["duration"] = static_cast<double>(obj.particles.duration);
        jprt["maxLimit"] = obj.particles.maxLimit;
        jprt["primitiveSource"] = obj.particles.primitiveSource;
        jprt["direction"] = vec2ToJson(obj.particles.direction);
        jprt["startSize"] = vec2ToJson(obj.particles.startSize);
        jprt["endSize"] = vec2ToJson(obj.particles.endSize);
        jprt["startColor"] = colorToJson(obj.particles.startColor);
        jprt["endColor"] = colorToJson(obj.particles.endColor);
        jo["particleSystem"] = jprt;

        // Physics & MoveController2D
        json jphy = json::object();
        jphy["componentId"] = obj.physics.componentId;
        jphy["moveControllerEnabled"] = obj.physics.moveControllerEnabled;
        jphy["moveSpeed"] = static_cast<double>(obj.physics.moveSpeed);
        jphy["colliderEnabled"] = obj.physics.colliderEnabled;
        jphy["colliderType"] = obj.physics.colliderType;
        jphy["colliderSize"] = vec2ToJson(obj.physics.colliderSize);
        jphy["colliderRadius"] = static_cast<double>(obj.physics.colliderRadius);
        jphy["collisionLayer"] = obj.physics.collisionLayer;
        jo["physics2D"] = jphy;

        // Rigidbody2D
        json jrb = json::object();
        jrb["componentId"] = obj.rigidbody2D.componentId;
        jrb["enabled"] = obj.rigidbody2D.enabled;
        jrb["bodyType"] = obj.rigidbody2D.bodyType;
        jrb["mass"] = static_cast<double>(obj.rigidbody2D.mass);
        jrb["linearDamping"] = static_cast<double>(obj.rigidbody2D.linearDamping);
        jrb["angularDamping"] = static_cast<double>(obj.rigidbody2D.angularDamping);
        jrb["gravityScale"] = static_cast<double>(obj.rigidbody2D.gravityScale);
        jrb["freezeRotation"] = obj.rigidbody2D.freezeRotation;
        jrb["simulated"] = obj.rigidbody2D.simulated;
        jrb["velocity"] = vec2ToJson(obj.rigidbody2D.velocity);
        jrb["angularVelocity"] = static_cast<double>(obj.rigidbody2D.angularVelocity);
        jo["rigidbody2D"] = jrb;

        // BoxCollider2D
        json jbox = json::object();
        jbox["componentId"] = obj.boxCollider2D.componentId;
        jbox["enabled"] = obj.boxCollider2D.enabled;
        jbox["size"] = vec2ToJson(obj.boxCollider2D.size);
        jbox["offset"] = vec2ToJson(obj.boxCollider2D.offset);
        jbox["isTrigger"] = obj.boxCollider2D.isTrigger;
        jbox["density"] = static_cast<double>(obj.boxCollider2D.density);
        jbox["friction"] = static_cast<double>(obj.boxCollider2D.friction);
        jbox["bounciness"] = static_cast<double>(obj.boxCollider2D.bounciness);
        jo["boxCollider2D"] = jbox;

        // MultiCollider2D
        json jmulti = json::object();
        jmulti["componentId"] = obj.multiCollider2D.componentId;
        jmulti["enabled"] = obj.multiCollider2D.enabled;
        json jpoints = json::array();
        for (const auto &pt : obj.multiCollider2D.points)
        {
            jpoints.push_back(vec2ToJson(pt));
        }
        jmulti["points"] = jpoints;
        jmulti["offset"] = vec2ToJson(obj.multiCollider2D.offset);
        jmulti["isTrigger"] = obj.multiCollider2D.isTrigger;
        jmulti["density"] = static_cast<double>(obj.multiCollider2D.density);
        jmulti["friction"] = static_cast<double>(obj.multiCollider2D.friction);
        jmulti["bounciness"] = static_cast<double>(obj.multiCollider2D.bounciness);
        jmulti["isClosed"] = obj.multiCollider2D.isClosed;
        jo["multiCollider2D"] = jmulti;

        // NavMeshAgent
        json jagent = json::object();
        jagent["componentId"] = obj.navMeshAgent.componentId;
        jagent["enabled"] = obj.navMeshAgent.enabled;
        jagent["speed"] = static_cast<double>(obj.navMeshAgent.speed);
        jagent["angularSpeed"] = static_cast<double>(obj.navMeshAgent.angularSpeed);
        jagent["acceleration"] = static_cast<double>(obj.navMeshAgent.acceleration);
        jagent["stoppingDistance"] = static_cast<double>(obj.navMeshAgent.stoppingDistance);
        jagent["radius"] = static_cast<double>(obj.navMeshAgent.radius);
        jagent["destination"] = vec2ToJson(obj.navMeshAgent.destination);
        jagent["hasDestination"] = obj.navMeshAgent.hasDestination;
        jagent["updateRotation"] = obj.navMeshAgent.updateRotation;
        jagent["autoRepath"] = obj.navMeshAgent.autoRepath;
        jo["navMeshAgent"] = jagent;

        // AudioSource
        json jaud = json::object();
        jaud["componentId"] = obj.audio.componentId;
        jaud["clipAssetId"] = static_cast<long>(obj.audio.clipAssetId);
        jaud["enabled"] = obj.audio.enabled;
        jaud["volume"] = static_cast<double>(obj.audio.volume);
        jaud["loop"] = obj.audio.loop;
        jaud["playOnAwake"] = obj.audio.playOnAwake;
        jaud["clipPath"] = obj.audio.clipPath;
        jo["audioSource"] = jaud;

        // MotionScript
        json jscr = json::object();
        jscr["componentId"] = obj.script.componentId;
        jscr["enabled"] = obj.script.enabled;
        jscr["linearVelocity"] = vec2ToJson(obj.script.linearVelocity);
        jscr["angularVelocity"] = static_cast<double>(obj.script.angularVelocity);
        jscr["pulseSpeed"] = static_cast<double>(obj.script.pulseSpeed);
        jscr["pulseAmplitude"] = static_cast<double>(obj.script.pulseAmplitude);
        jo["motionScript"] = jscr;

        return jo;
    }

    static json serializeWorldToJson(const EditorWorld &w)
    {
        json jw = json::object();
        jw["name"] = w.name;
        jw["ambientColor"] = colorToJson(w.ambientColor);

        // NavMesh
        json jnav = json::object();
        jnav["width"] = w.navMesh.width;
        jnav["height"] = w.navMesh.height;
        jnav["heuristicMethod"] = w.navMesh.heuristicMethod;
        jnav["identityMethod"] = w.navMesh.identityMethod;
        jnav["mazeAlgorithm"] = w.navMesh.mazeAlgorithm;
        jnav["startX"] = w.navMesh.start.x;
        jnav["startY"] = w.navMesh.start.y;
        jnav["goalX"] = w.navMesh.goal.x;
        jnav["goalY"] = w.navMesh.goal.y;
        jw["navMesh"] = jnav;

        // GameObjects
        json jobjects = json::array();
        for (const auto &obj : w.objects)
        {
            jobjects.push_back(serializeObjectToJson(obj));
        }
        jw["objects"] = jobjects;

        // UI::GUI Elements
        json jgui = json::array();
        for (const auto &ui : w.guiElements)
        {
            json ju = json::object();
            ju["id"] = ui.id;
            ju["name"] = ui.name;
            ju["type"] = ui.type;
            ju["x"] = ui.rect.x;
            ju["y"] = ui.rect.y;
            ju["w"] = ui.rect.w;
            ju["h"] = ui.rect.h;
            ju["text"] = ui.text;
            ju["fontSize"] = ui.fontSize;
            ju["visible"] = ui.visible;
            ju["interactable"] = ui.interactable;
            ju["boolValue"] = ui.boolValue;
            ju["floatValue"] = static_cast<double>(ui.floatValue);
            ju["minValue"] = static_cast<double>(ui.minValue);
            ju["maxValue"] = static_cast<double>(ui.maxValue);
            ju["selectedIndex"] = ui.selectedIndex;
            ju["itemsCsv"] = ui.itemsCsv;
            ju["targetWorldOnClick"] = ui.targetWorldOnClick;
            jgui.push_back(ju);
        }
        jw["guiElements"] = jgui;

        return jw;
    }

    static void deserializeObjectFromJson(const json &jo, EditorObject &obj)
    {
        if (jo.contains("id"))
            obj.id = jsonToInt(jo.at("id"), 1);
        if (jo.contains("name") && jo.at("name").is_string())
            obj.name = jo.at("name").get<std::string>();
        if (jo.contains("tag") && jo.at("tag").is_string())
            obj.tag = jo.at("tag").get<std::string>();
        if (jo.contains("layer"))
            obj.layer = jsonToInt(jo.at("layer"), 0);
        if (jo.contains("active"))
            obj.active = jsonToBool(jo.at("active"), true);
        if (jo.contains("parentId"))
            obj.parentId = jsonToInt(jo.at("parentId"), -1);
        if (jo.contains("position"))
            obj.position = jsonToVec2(jo.at("position"), obj.position);
        if (jo.contains("angle"))
            obj.angle = jsonToFloat(jo.at("angle"), 0.0f);
        if (jo.contains("scale"))
            obj.scale = jsonToVec2(jo.at("scale"), obj.scale);
        if (jo.contains("zOrder"))
            obj.zOrder = jsonToInt(jo.at("zOrder"), 0);

        if (jo.contains("transform") && jo.at("transform").is_object())
        {
            const json &jt = jo.at("transform");
            if (jt.contains("parentId"))
                obj.parentId = jsonToInt(jt.at("parentId"), obj.parentId);
            if (jt.contains("position"))
                obj.position = jsonToVec2(jt.at("position"), obj.position);
            if (jt.contains("angle"))
                obj.angle = jsonToFloat(jt.at("angle"), obj.angle);
            if (jt.contains("scale"))
                obj.scale = jsonToVec2(jt.at("scale"), obj.scale);
        }

        if (jo.contains("components") && jo.at("components").is_array())
        {
            for (const auto &jc : jo.at("components"))
            {
                if (!jc.is_object() || !jc.contains("type") || !jc.at("type").is_string())
                    continue;
                std::string compType = jc.at("type").get<std::string>();
                if (compType == "Camera2D" || compType == "Camera")
                {
                    obj.camera.enabled = true;
                    if (jc.contains("mainCamera")) obj.camera.mainCamera = jsonToBool(jc.at("mainCamera"), true);
                    if (jc.contains("zoom")) obj.camera.zoom = jsonToFloat(jc.at("zoom"), 1.0f);
                    if (jc.contains("backcolor")) obj.camera.backcolor = jsonToColor(jc.at("backcolor"), obj.camera.backcolor);
                }
                else if (compType == "Light" || compType == "Light2D" || compType == "SimpleLight" || compType == "DirectLight")
                {
                    obj.light.enabled = true;
                    if (compType == "DirectLight") obj.light.lightType = 1;
                    else if (compType == "Spotlight") obj.light.lightType = 2;
                    else obj.light.lightType = 0;
                    if (jc.contains("color")) obj.light.color = jsonToColor(jc.at("color"), obj.light.color);
                    if (jc.contains("range")) obj.light.range = jsonToFloat(jc.at("range"), 3.5f);
                    if (jc.contains("intensity")) obj.light.intensity = jsonToFloat(jc.at("intensity"), 1.0f);
                }
                else if (compType == "SpriteRenderer")
                {
                    obj.sprite.enabled = true;
                    if (jc.contains("size")) obj.sprite.size = jsonToVec2(jc.at("size"), obj.sprite.size);
                    if (jc.contains("color")) obj.sprite.color = jsonToColor(jc.at("color"), obj.sprite.color);
                    if (jc.contains("primitiveShape") && jc.at("primitiveShape").is_string())
                    {
                        std::string shape = jc.at("primitiveShape").get<std::string>();
                        if (shape == "Circle") obj.sprite.primitiveShape = 1;
                        else if (shape == "Triangle") obj.sprite.primitiveShape = 2;
                        else obj.sprite.primitiveShape = 0;
                    }
                }
                else if (compType == "Terrain2D")
                {
                    obj.terrain.enabled = true;
                    if (jc.contains("width")) obj.terrain.width = jsonToInt(jc.at("width"), 12);
                    if (jc.contains("length")) obj.terrain.length = jsonToInt(jc.at("length"), 8);
                    if (jc.contains("cellScale")) obj.terrain.cellScale = jsonToFloat(jc.at("cellScale"), 1.0f);
                    obj.terrain.ensureGridInitialized(0);
                }
                else if (compType == "AudioSource")
                {
                    obj.audio.enabled = true;
                    if (jc.contains("volume")) obj.audio.volume = jsonToFloat(jc.at("volume"), 0.85f);
                    if (jc.contains("loop")) obj.audio.loop = jsonToBool(jc.at("loop"), true);
                }
                else if (compType == "Physics2D" || compType == "Collider2D" || compType == "BoxCollider2D")
                {
                    obj.physics.colliderEnabled = true;
                }
            }
        }

        if (jo.contains("spriteRenderer") && jo.at("spriteRenderer").is_object())
        {
            const json &js = jo.at("spriteRenderer");
            if (js.contains("componentId"))
                obj.sprite.componentId = jsonToInt(js.at("componentId"));
            if (js.contains("assetId"))
                obj.sprite.assetId = jsonToUint32(js.at("assetId"));
            if (js.contains("enabled"))
                obj.sprite.enabled = jsonToBool(js.at("enabled"));
            if (js.contains("primitiveShape"))
                obj.sprite.primitiveShape = jsonToInt(js.at("primitiveShape"));
            if (js.contains("size"))
                obj.sprite.size = jsonToVec2(js.at("size"), obj.sprite.size);
            if (js.contains("pivot"))
                obj.sprite.pivot = jsonToVec2(js.at("pivot"), obj.sprite.pivot);
            if (js.contains("color"))
                obj.sprite.color = jsonToColor(js.at("color"), obj.sprite.color);
            if (js.contains("renderType"))
                obj.sprite.renderType = jsonToInt(js.at("renderType"));
            if (js.contains("renderOut"))
                obj.sprite.renderOut = jsonToInt(js.at("renderOut"));
            if (js.contains("presentMode"))
                obj.sprite.presentMode = jsonToInt(js.at("presentMode"));
            if (js.contains("flip"))
                obj.sprite.flip = jsonToInt(js.at("flip"));
            if (js.contains("spritePath") && js.at("spritePath").is_string())
                obj.sprite.spritePath = js.at("spritePath").get<std::string>();
        }

        if (jo.contains("camera2D") && jo.at("camera2D").is_object())
        {
            const json &jc = jo.at("camera2D");
            if (jc.contains("componentId"))
                obj.camera.componentId = jsonToInt(jc.at("componentId"));
            if (jc.contains("enabled"))
                obj.camera.enabled = jsonToBool(jc.at("enabled"));
            if (jc.contains("mainCamera"))
                obj.camera.mainCamera = jsonToBool(jc.at("mainCamera"), true);
            if (jc.contains("backclear"))
                obj.camera.backclear = jsonToBool(jc.at("backclear"), true);
            if (jc.contains("backcolor"))
                obj.camera.backcolor = jsonToColor(jc.at("backcolor"), obj.camera.backcolor);
            if (jc.contains("zoom"))
                obj.camera.zoom = jsonToFloat(jc.at("zoom"), 1.0f);
            if (jc.contains("scale"))
                obj.camera.scale = jsonToVec2(jc.at("scale"), RoninEngine::Runtime::Vec2(obj.camera.zoom, obj.camera.zoom));
            else
                obj.camera.scale = RoninEngine::Runtime::Vec2(obj.camera.zoom, obj.camera.zoom);
            if (jc.contains("zoomOut"))
                obj.camera.zoomOut = jsonToFloat(jc.at("zoomOut"), 100.0f);
            if (jc.contains("distanceEvcall"))
                obj.camera.distanceEvcall = jsonToInt(jc.at("distanceEvcall"));
            if (jc.contains("visibleBorders"))
                obj.camera.visibleBorders = jsonToBool(jc.at("visibleBorders"));
            if (jc.contains("visibleGrids"))
                obj.camera.visibleGrids = jsonToBool(jc.at("visibleGrids"));
            if (jc.contains("visibleObjects"))
                obj.camera.visibleObjects = jsonToBool(jc.at("visibleObjects"), true);
            if (jc.contains("visibleNames"))
                obj.camera.visibleNames = jsonToBool(jc.at("visibleNames"));
            if (jc.contains("cullResources"))
                obj.camera.cullResources = jsonToBool(jc.at("cullResources"), true);
        }

        if (jo.contains("light2D") && jo.at("light2D").is_object())
        {
            const json &jl = jo.at("light2D");
            if (jl.contains("componentId"))
                obj.light.componentId = jsonToInt(jl.at("componentId"));
            if (jl.contains("enabled"))
                obj.light.enabled = jsonToBool(jl.at("enabled"));
            if (jl.contains("lightType"))
                obj.light.lightType = jsonToInt(jl.at("lightType"));
            if (jl.contains("intensity"))
                obj.light.intensity = jsonToFloat(jl.at("intensity"), 1.0f);
            if (jl.contains("range"))
                obj.light.range = jsonToFloat(jl.at("range"), 3.5f);
            if (jl.contains("angleDegrees"))
                obj.light.angleDegrees = jsonToFloat(jl.at("angleDegrees"), 45.0f);
            if (jl.contains("color"))
                obj.light.color = jsonToColor(jl.at("color"), obj.light.color);
        }

        if (jo.contains("terrain2D") && jo.at("terrain2D").is_object())
        {
            deserializeTerrainFromJson(jo.at("terrain2D"), obj.terrain);
            if (obj.terrain.enabled)
                obj.terrain.ensureGridInitialized(0);
        }

        if (jo.contains("particleSystem") && jo.at("particleSystem").is_object())
        {
            const json &jp = jo.at("particleSystem");
            if (jp.contains("componentId"))
                obj.particles.componentId = jsonToInt(jp.at("componentId"));
            if (jp.contains("enabled"))
                obj.particles.enabled = jsonToBool(jp.at("enabled"));
            if (jp.contains("emit"))
                obj.particles.emit = jsonToBool(jp.at("emit"), true);
            if (jp.contains("loop"))
                obj.particles.loop = jsonToBool(jp.at("loop"), true);
            if (jp.contains("rotate"))
                obj.particles.rotate = jsonToBool(jp.at("rotate"));
            if (jp.contains("worldSpace"))
                obj.particles.worldSpace = jsonToBool(jp.at("worldSpace"));
            if (jp.contains("randomDirection"))
                obj.particles.randomDirection = jsonToBool(jp.at("randomDirection"));
            if (jp.contains("interval"))
                obj.particles.interval = jsonToFloat(jp.at("interval"), 0.05f);
            if (jp.contains("speed"))
                obj.particles.speed = jsonToFloat(jp.at("speed"), 1.8f);
            if (jp.contains("duration"))
                obj.particles.duration = jsonToFloat(jp.at("duration"), 1.6f);
            if (jp.contains("maxLimit"))
                obj.particles.maxLimit = jsonToInt(jp.at("maxLimit"), 64);
            if (jp.contains("primitiveSource"))
                obj.particles.primitiveSource = jsonToInt(jp.at("primitiveSource"), 0);
            if (jp.contains("direction"))
                obj.particles.direction = jsonToVec2(jp.at("direction"), obj.particles.direction);
            if (jp.contains("startSize"))
                obj.particles.startSize = jsonToVec2(jp.at("startSize"), obj.particles.startSize);
            if (jp.contains("endSize"))
                obj.particles.endSize = jsonToVec2(jp.at("endSize"), obj.particles.endSize);
            if (jp.contains("startColor"))
                obj.particles.startColor = jsonToColor(jp.at("startColor"), obj.particles.startColor);
            if (jp.contains("endColor"))
                obj.particles.endColor = jsonToColor(jp.at("endColor"), obj.particles.endColor);
        }

        if (jo.contains("physics2D") && jo.at("physics2D").is_object())
        {
            const json &jph = jo.at("physics2D");
            if (jph.contains("componentId"))
                obj.physics.componentId = jsonToInt(jph.at("componentId"));
            if (jph.contains("moveControllerEnabled"))
                obj.physics.moveControllerEnabled = jsonToBool(jph.at("moveControllerEnabled"));
            if (jph.contains("moveSpeed"))
                obj.physics.moveSpeed = jsonToFloat(jph.at("moveSpeed"), 3.5f);
            if (jph.contains("colliderEnabled"))
                obj.physics.colliderEnabled = jsonToBool(jph.at("colliderEnabled"));
            if (jph.contains("colliderType"))
                obj.physics.colliderType = jsonToInt(jph.at("colliderType"));
            if (jph.contains("colliderSize"))
                obj.physics.colliderSize = jsonToVec2(jph.at("colliderSize"), obj.physics.colliderSize);
            if (jph.contains("colliderRadius"))
                obj.physics.colliderRadius = jsonToFloat(jph.at("colliderRadius"), 0.5f);
            if (jph.contains("collisionLayer"))
                obj.physics.collisionLayer = jsonToInt(jph.at("collisionLayer"), 0);
        }

        if (jo.contains("rigidbody2D") && jo.at("rigidbody2D").is_object())
        {
            const json &jrb = jo.at("rigidbody2D");
            if (jrb.contains("componentId"))
                obj.rigidbody2D.componentId = jsonToInt(jrb.at("componentId"));
            if (jrb.contains("enabled"))
                obj.rigidbody2D.enabled = jsonToBool(jrb.at("enabled"));
            if (jrb.contains("bodyType"))
                obj.rigidbody2D.bodyType = jsonToInt(jrb.at("bodyType"), 0);
            if (jrb.contains("mass"))
                obj.rigidbody2D.mass = jsonToFloat(jrb.at("mass"), 1.0f);
            if (jrb.contains("linearDamping"))
                obj.rigidbody2D.linearDamping = jsonToFloat(jrb.at("linearDamping"), 0.0f);
            if (jrb.contains("angularDamping"))
                obj.rigidbody2D.angularDamping = jsonToFloat(jrb.at("angularDamping"), 0.05f);
            if (jrb.contains("gravityScale"))
                obj.rigidbody2D.gravityScale = jsonToFloat(jrb.at("gravityScale"), 1.0f);
            if (jrb.contains("freezeRotation"))
                obj.rigidbody2D.freezeRotation = jsonToBool(jrb.at("freezeRotation"), false);
            if (jrb.contains("simulated"))
                obj.rigidbody2D.simulated = jsonToBool(jrb.at("simulated"), true);
            if (jrb.contains("velocity"))
                obj.rigidbody2D.velocity = jsonToVec2(jrb.at("velocity"), obj.rigidbody2D.velocity);
            if (jrb.contains("angularVelocity"))
                obj.rigidbody2D.angularVelocity = jsonToFloat(jrb.at("angularVelocity"), 0.0f);
        }

        if (jo.contains("boxCollider2D") && jo.at("boxCollider2D").is_object())
        {
            const json &jbox = jo.at("boxCollider2D");
            if (jbox.contains("componentId"))
                obj.boxCollider2D.componentId = jsonToInt(jbox.at("componentId"));
            if (jbox.contains("enabled"))
                obj.boxCollider2D.enabled = jsonToBool(jbox.at("enabled"));
            if (jbox.contains("size"))
                obj.boxCollider2D.size = jsonToVec2(jbox.at("size"), obj.boxCollider2D.size);
            if (jbox.contains("offset"))
                obj.boxCollider2D.offset = jsonToVec2(jbox.at("offset"), obj.boxCollider2D.offset);
            if (jbox.contains("isTrigger"))
                obj.boxCollider2D.isTrigger = jsonToBool(jbox.at("isTrigger"), false);
            if (jbox.contains("density"))
                obj.boxCollider2D.density = jsonToFloat(jbox.at("density"), 1.0f);
            if (jbox.contains("friction"))
                obj.boxCollider2D.friction = jsonToFloat(jbox.at("friction"), 0.3f);
            if (jbox.contains("bounciness"))
                obj.boxCollider2D.bounciness = jsonToFloat(jbox.at("bounciness"), 0.0f);
        }

        if (jo.contains("multiCollider2D") && jo.at("multiCollider2D").is_object())
        {
            const json &jmulti = jo.at("multiCollider2D");
            if (jmulti.contains("componentId"))
                obj.multiCollider2D.componentId = jsonToInt(jmulti.at("componentId"));
            if (jmulti.contains("enabled"))
                obj.multiCollider2D.enabled = jsonToBool(jmulti.at("enabled"));
            if (jmulti.contains("points") && jmulti.at("points").is_array())
            {
                obj.multiCollider2D.points.clear();
                for (const auto &item : jmulti.at("points"))
                {
                    obj.multiCollider2D.points.push_back(jsonToVec2(item, {0.0f, 0.0f}));
                }
            }
            if (jmulti.contains("offset"))
                obj.multiCollider2D.offset = jsonToVec2(jmulti.at("offset"), obj.multiCollider2D.offset);
            if (jmulti.contains("isTrigger"))
                obj.multiCollider2D.isTrigger = jsonToBool(jmulti.at("isTrigger"), false);
            if (jmulti.contains("density"))
                obj.multiCollider2D.density = jsonToFloat(jmulti.at("density"), 1.0f);
            if (jmulti.contains("friction"))
                obj.multiCollider2D.friction = jsonToFloat(jmulti.at("friction"), 0.4f);
            if (jmulti.contains("bounciness"))
                obj.multiCollider2D.bounciness = jsonToFloat(jmulti.at("bounciness"), 0.1f);
            if (jmulti.contains("isClosed"))
                obj.multiCollider2D.isClosed = jsonToBool(jmulti.at("isClosed"), true);
        }

        if (jo.contains("navMeshAgent") && jo.at("navMeshAgent").is_object())
        {
            const json &jagent = jo.at("navMeshAgent");
            if (jagent.contains("componentId"))
                obj.navMeshAgent.componentId = jsonToInt(jagent.at("componentId"));
            if (jagent.contains("enabled"))
                obj.navMeshAgent.enabled = jsonToBool(jagent.at("enabled"));
            if (jagent.contains("speed"))
                obj.navMeshAgent.speed = jsonToFloat(jagent.at("speed"), 3.5f);
            if (jagent.contains("angularSpeed"))
                obj.navMeshAgent.angularSpeed = jsonToFloat(jagent.at("angularSpeed"), 120.0f);
            if (jagent.contains("acceleration"))
                obj.navMeshAgent.acceleration = jsonToFloat(jagent.at("acceleration"), 8.0f);
            if (jagent.contains("stoppingDistance"))
                obj.navMeshAgent.stoppingDistance = jsonToFloat(jagent.at("stoppingDistance"), 0.25f);
            if (jagent.contains("radius"))
                obj.navMeshAgent.radius = jsonToFloat(jagent.at("radius"), 0.4f);
            if (jagent.contains("destination"))
                obj.navMeshAgent.destination = jsonToVec2(jagent.at("destination"), obj.navMeshAgent.destination);
            if (jagent.contains("hasDestination"))
                obj.navMeshAgent.hasDestination = jsonToBool(jagent.at("hasDestination"), false);
            if (jagent.contains("updateRotation"))
                obj.navMeshAgent.updateRotation = jsonToBool(jagent.at("updateRotation"), false);
            if (jagent.contains("autoRepath"))
                obj.navMeshAgent.autoRepath = jsonToBool(jagent.at("autoRepath"), true);
        }

        if (jo.contains("audioSource") && jo.at("audioSource").is_object())
        {
            const json &ja = jo.at("audioSource");
            if (ja.contains("componentId"))
                obj.audio.componentId = jsonToInt(ja.at("componentId"));
            if (ja.contains("clipAssetId"))
                obj.audio.clipAssetId = jsonToUint32(ja.at("clipAssetId"));
            if (ja.contains("enabled"))
                obj.audio.enabled = jsonToBool(ja.at("enabled"));
            if (ja.contains("volume"))
                obj.audio.volume = jsonToFloat(ja.at("volume"), 0.85f);
            if (ja.contains("loop"))
                obj.audio.loop = jsonToBool(ja.at("loop"), true);
            if (ja.contains("playOnAwake"))
                obj.audio.playOnAwake = jsonToBool(ja.at("playOnAwake"), true);
            if (ja.contains("clipPath") && ja.at("clipPath").is_string())
                obj.audio.clipPath = ja.at("clipPath").get<std::string>();
        }

        if (jo.contains("motionScript") && jo.at("motionScript").is_object())
        {
            const json &jm = jo.at("motionScript");
            if (jm.contains("componentId"))
                obj.script.componentId = jsonToInt(jm.at("componentId"));
            if (jm.contains("enabled"))
                obj.script.enabled = jsonToBool(jm.at("enabled"));
            if (jm.contains("linearVelocity"))
                obj.script.linearVelocity = jsonToVec2(jm.at("linearVelocity"), obj.script.linearVelocity);
            if (jm.contains("angularVelocity"))
                obj.script.angularVelocity = jsonToFloat(jm.at("angularVelocity"));
            if (jm.contains("pulseSpeed"))
                obj.script.pulseSpeed = jsonToFloat(jm.at("pulseSpeed"));
            if (jm.contains("pulseAmplitude"))
                obj.script.pulseAmplitude = jsonToFloat(jm.at("pulseAmplitude"));
        }
    }

    static EditorWorld deserializeWorldFromJson(const json &jw)
    {
        EditorWorld w;
        initDefaultNavMesh(w.navMesh);

        if (jw.contains("name") && jw.at("name").is_string())
            w.name = jw.at("name").get<std::string>();
        if (jw.contains("ambientColor"))
            w.ambientColor = jsonToColor(jw.at("ambientColor"), w.ambientColor);

        if (jw.contains("navMesh") && jw.at("navMesh").is_object())
        {
            const json &jn = jw.at("navMesh");
            if (jn.contains("width"))
                w.navMesh.width = jsonToInt(jn.at("width"), 16);
            if (jn.contains("height"))
                w.navMesh.height = jsonToInt(jn.at("height"), 10);
            if (jn.contains("heuristicMethod"))
                w.navMesh.heuristicMethod = jsonToInt(jn.at("heuristicMethod"), 0);
            if (jn.contains("identityMethod"))
                w.navMesh.identityMethod = jsonToInt(jn.at("identityMethod"), 0);
            if (jn.contains("mazeAlgorithm"))
                w.navMesh.mazeAlgorithm = jsonToInt(jn.at("mazeAlgorithm"), 0);
            if (jn.contains("startX"))
                w.navMesh.start.x = jsonToInt(jn.at("startX"), 1);
            if (jn.contains("startY"))
                w.navMesh.start.y = jsonToInt(jn.at("startY"), 1);
            if (jn.contains("goalX"))
                w.navMesh.goal.x = jsonToInt(jn.at("goalX"), 14);
            if (jn.contains("goalY"))
                w.navMesh.goal.y = jsonToInt(jn.at("goalY"), 8);
            initDefaultNavMesh(w.navMesh);
        }

        int maxObjId = 0;
        if (jw.contains("objects") && jw.at("objects").is_array())
        {
            for (const auto &jo : jw.at("objects"))
            {
                if (!jo.is_object())
                    continue;
                EditorObject obj;
                deserializeObjectFromJson(jo, obj);

                // Ensure all enabled components have valid component IDs
                if (obj.sprite.enabled && obj.sprite.componentId == 0) obj.sprite.componentId = w.allocateComponentId();
                if (obj.camera.enabled && obj.camera.componentId == 0) obj.camera.componentId = w.allocateComponentId();
                if (obj.light.enabled && obj.light.componentId == 0) obj.light.componentId = w.allocateComponentId();
                if (obj.terrain.enabled && obj.terrain.componentId == 0) obj.terrain.componentId = w.allocateComponentId();
                if (obj.particles.enabled && obj.particles.componentId == 0) obj.particles.componentId = w.allocateComponentId();
                if (obj.physics.colliderEnabled && obj.physics.componentId == 0) obj.physics.componentId = w.allocateComponentId();
                if (obj.rigidbody2D.enabled && obj.rigidbody2D.componentId == 0) obj.rigidbody2D.componentId = w.allocateComponentId();
                if (obj.boxCollider2D.enabled && obj.boxCollider2D.componentId == 0) obj.boxCollider2D.componentId = w.allocateComponentId();
                if (obj.multiCollider2D.enabled && obj.multiCollider2D.componentId == 0) obj.multiCollider2D.componentId = w.allocateComponentId();
                if (obj.navMeshAgent.enabled && obj.navMeshAgent.componentId == 0) obj.navMeshAgent.componentId = w.allocateComponentId();
                if (obj.audio.enabled && obj.audio.componentId == 0) obj.audio.componentId = w.allocateComponentId();
                if (obj.script.enabled && obj.script.componentId == 0) obj.script.componentId = w.allocateComponentId();

                maxObjId = std::max(maxObjId, obj.id);
                w.objects.push_back(obj);
            }
        }
        w.nextObjectId = maxObjId + 1;

        int maxUIId = 0;
        if (jw.contains("guiElements") && jw.at("guiElements").is_array())
        {
            for (const auto &ju : jw.at("guiElements"))
            {
                if (!ju.is_object())
                    continue;
                EditorUIElement ui;
                if (ju.contains("id"))
                    ui.id = jsonToInt(ju.at("id"), 1);
                if (ju.contains("name") && ju.at("name").is_string())
                    ui.name = ju.at("name").get<std::string>();
                if (ju.contains("type"))
                    ui.type = jsonToInt(ju.at("type"), 0);
                if (ju.contains("x"))
                    ui.rect.x = jsonToInt(ju.at("x"), 24);
                if (ju.contains("y"))
                    ui.rect.y = jsonToInt(ju.at("y"), 24);
                if (ju.contains("w"))
                    ui.rect.w = jsonToInt(ju.at("w"), 220);
                if (ju.contains("h"))
                    ui.rect.h = jsonToInt(ju.at("h"), 32);
                if (ju.contains("text") && ju.at("text").is_string())
                    ui.text = ju.at("text").get<std::string>();
                if (ju.contains("fontSize"))
                    ui.fontSize = jsonToInt(ju.at("fontSize"), 13);
                if (ju.contains("visible"))
                    ui.visible = jsonToBool(ju.at("visible"), true);
                if (ju.contains("interactable"))
                    ui.interactable = jsonToBool(ju.at("interactable"), true);
                if (ju.contains("boolValue"))
                    ui.boolValue = jsonToBool(ju.at("boolValue"), false);
                if (ju.contains("floatValue"))
                    ui.floatValue = jsonToFloat(ju.at("floatValue"), 0.75f);
                if (ju.contains("minValue"))
                    ui.minValue = jsonToFloat(ju.at("minValue"), 0.0f);
                if (ju.contains("maxValue"))
                    ui.maxValue = jsonToFloat(ju.at("maxValue"), 1.0f);
                if (ju.contains("selectedIndex"))
                    ui.selectedIndex = jsonToInt(ju.at("selectedIndex"), 0);
                if (ju.contains("itemsCsv") && ju.at("itemsCsv").is_string())
                    ui.itemsCsv = ju.at("itemsCsv").get<std::string>();
                if (ju.contains("targetWorldOnClick") && ju.at("targetWorldOnClick").is_string())
                    ui.targetWorldOnClick = ju.at("targetWorldOnClick").get<std::string>();

                maxUIId = std::max(maxUIId, ui.id);
                w.guiElements.push_back(ui);
            }
        }
        w.nextUIElementId = maxUIId + 1;

        return w;
    }

    bool EditorState::saveProjectToFile(const std::string &filePath)
    {
        std::setlocale(LC_NUMERIC, "C");
        std::ofstream out(filePath);
        if (!out.is_open())
        {
            logError("Failed to open project JSON file for saving: " + filePath);
            return false;
        }

        json root = json::object();
        root["format"] = "RoninEngineProject";
        root["formatVersion"] = 2;
        root["editorVersion"] = getEditorVersion();
        root["engineVersion"] = getEngineVersion();
        root["projectName"] = project.projectName;
        root["author"] = project.author;
        root["resolutionWidth"] = project.resolutionWidth;
        root["resolutionHeight"] = project.resolutionHeight;
        root["refreshRate"] = project.refreshRate;
        root["fullscreen"] = project.fullscreen;
        root["verticalSync"] = project.verticalSync;
        root["renderBackend"] = project.renderBackend;
        root["textureQuality"] = project.textureQuality;
        root["brightness"] = static_cast<double>(project.brightness);
        root["windowOpacity"] = static_cast<double>(project.windowOpacity);
        root["debugMode"] = project.debugMode;
        root["startupWorldIndex"] = project.startupWorldIndex;
        root["activeWorldIndex"] = activeWorldIndex;

        json jworlds = json::array();
        for (const auto &w : project.worlds)
        {
            jworlds.push_back(serializeWorldToJson(w));
        }
        root["worlds"] = jworlds;

        out << root.dump(2) << "\n";
        project.projectFilePath = filePath;
        isDirty = false;
        logInfo("Saved Project '" + project.projectName + "' (" +
                std::to_string(project.worlds.size()) + " Worlds) as JSON to " + filePath);
        return true;
    }

    bool EditorState::loadProjectFromFile(const std::string &filePath)
    {
        std::setlocale(LC_NUMERIC, "C");
        std::ifstream in(filePath);
        if (!in.is_open())
        {
            logError("Failed to open project JSON file: " + filePath);
            return false;
        }

        std::stringstream buffer;
        buffer << in.rdbuf();
        json root = json::parse(buffer.str(), nullptr, false);
        if (root.is_discarded() || !root.is_object())
        {
            logError("Invalid JSON in project file: " + filePath);
            return false;
        }

        EditorProject loadedProj;
        loadedProj.projectFilePath = filePath;
        if (root.contains("projectName") && root.at("projectName").is_string())
            loadedProj.projectName = root.at("projectName").get<std::string>();
        if (root.contains("author") && root.at("author").is_string())
            loadedProj.author = root.at("author").get<std::string>();
        if (root.contains("resolutionWidth"))
            loadedProj.resolutionWidth = jsonToInt(root.at("resolutionWidth"), 1280);
        if (root.contains("resolutionHeight"))
            loadedProj.resolutionHeight = jsonToInt(root.at("resolutionHeight"), 720);
        if (root.contains("refreshRate"))
            loadedProj.refreshRate = jsonToInt(root.at("refreshRate"), 60);
        if (root.contains("fullscreen"))
            loadedProj.fullscreen = jsonToBool(root.at("fullscreen"), false);
        if (root.contains("verticalSync"))
            loadedProj.verticalSync = jsonToBool(root.at("verticalSync"), true);
        if (root.contains("renderBackend"))
            loadedProj.renderBackend = jsonToInt(root.at("renderBackend"), 0);
        if (root.contains("textureQuality"))
            loadedProj.textureQuality = jsonToInt(root.at("textureQuality"), 0);
        if (root.contains("brightness"))
            loadedProj.brightness = jsonToFloat(root.at("brightness"), 1.0f);
        if (root.contains("windowOpacity"))
            loadedProj.windowOpacity = jsonToFloat(root.at("windowOpacity"), 1.0f);
        if (root.contains("debugMode"))
            loadedProj.debugMode = jsonToBool(root.at("debugMode"), false);
        if (root.contains("startupWorldIndex"))
            loadedProj.startupWorldIndex = jsonToInt(root.at("startupWorldIndex"), 0);

        int loadedActiveWorld = root.contains("activeWorldIndex")
                                    ? jsonToInt(root.at("activeWorldIndex"), 0)
                                    : 0;

        if (root.contains("worlds") && root.at("worlds").is_array())
        {
            for (const auto &jw : root.at("worlds"))
            {
                if (jw.is_object())
                {
                    loadedProj.worlds.push_back(deserializeWorldFromJson(jw));
                }
            }
        }

        if (loadedProj.worlds.empty())
        {
            logError("Project JSON file contained no Worlds: " + filePath);
            return false;
        }

        project = std::move(loadedProj);
        activeWorldIndex = std::clamp(loadedActiveWorld, 0, static_cast<int>(project.worlds.size()) - 1);
        selectedObjectId = activeWorld().objects.empty() ? -1 : activeWorld().objects.front().id;
        selectedUIElementId = activeWorld().guiElements.empty() ? -1 : activeWorld().guiElements.front().id;
        playState = EditorPlayState::Stopped;
        activeParticles.clear();
        isDirty = false;
        updateNavMeshPreview(true, false);
        runPhysicsStormCast();

        logInfo("Loaded Project '" + project.projectName + "' (" +
                std::to_string(project.worlds.size()) + " Worlds) from JSON " + filePath);
        return true;
    }

    bool EditorState::saveWorldToFile(const std::string &filePath, int worldIdx)
    {
        std::setlocale(LC_NUMERIC, "C");
        int idx = (worldIdx >= 0 && worldIdx < static_cast<int>(project.worlds.size()))
                      ? worldIdx
                      : activeWorldIndex;
        if (idx < 0 || idx >= static_cast<int>(project.worlds.size()))
            return false;

        std::ofstream out(filePath);
        if (!out.is_open())
        {
            logError("Failed to open World JSON file for saving: " + filePath);
            return false;
        }

        json jw = serializeWorldToJson(project.worlds[idx]);
        jw["format"] = "RoninEngineWorld";
        jw["editorVersion"] = getEditorVersion();
        out << jw.dump(2) << "\n";

        logInfo("Saved World '" + project.worlds[idx].name + "' as JSON to " + filePath);
        return true;
    }

    bool EditorState::loadWorldFromFile(const std::string &filePath)
    {
        std::setlocale(LC_NUMERIC, "C");
        std::ifstream in(filePath);
        if (!in.is_open())
        {
            logError("Failed to open World JSON file: " + filePath);
            return false;
        }

        std::stringstream buffer;
        buffer << in.rdbuf();
        json jw = json::parse(buffer.str(), nullptr, false);
        if (jw.is_discarded() || !jw.is_object())
        {
            logError("Invalid JSON in World file: " + filePath);
            return false;
        }

        EditorWorld w = deserializeWorldFromJson(jw);
        project.worlds.push_back(w);
        activeWorldIndex = static_cast<int>(project.worlds.size()) - 1;
        selectedObjectId = w.objects.empty() ? -1 : w.objects.front().id;
        selectedUIElementId = w.guiElements.empty() ? -1 : w.guiElements.front().id;
        isDirty = true;
        clearHistory();
        updateNavMeshPreview(false, false);
        runPhysicsStormCast();

        logInfo("Imported World '" + w.name + "' from JSON " + filePath);
        return true;
    }

    bool EditorState::savePrefab(const EditorObject &obj, const std::string &filePath)
    {
        std::setlocale(LC_NUMERIC, "C");
        std::filesystem::path p(filePath);
        std::error_code ec;
        if (!std::filesystem::exists(p.parent_path(), ec))
        {
            std::filesystem::create_directories(p.parent_path(), ec);
        }

        std::ofstream out(filePath);
        if (!out.is_open())
        {
            logError("Failed to open file for writing prefab: " + filePath);
            return false;
        }

        json jpref = json::object();
        jpref["format"] = "RoninEnginePrefab";
        jpref["prefabVersion"] = 1;
        jpref["name"] = obj.name;
        jpref["rootObject"] = serializeObjectToJson(obj);

        // Find child hierarchy in active world
        json jchildren = json::array();
        std::vector<int> parentIds = { obj.id };
        const auto &objs = activeWorld().objects;
        while (!parentIds.empty())
        {
            int currentParent = parentIds.back();
            parentIds.pop_back();

            for (const auto &cand : objs)
            {
                if (cand.parentId == currentParent && cand.id != obj.id)
                {
                    jchildren.push_back(serializeObjectToJson(cand));
                    parentIds.push_back(cand.id);
                }
            }
        }
        jpref["children"] = jchildren;

        out << jpref.dump(2) << "\n";
        out.close();

        // Ensure companion .meta file is generated
        MetaFile mf;
        MetaManager::EnsureMetaFile(filePath, mf);
        AssetDatabase::GetInstance().Refresh();

        logInfo("Saved Prefab '" + obj.name + "' to " + filePath);
        return true;
    }

    EditorObject *EditorState::instantiatePrefab(const std::string &filePath, const RoninEngine::Runtime::Vec2 &spawnPos, int parentId)
    {
        std::setlocale(LC_NUMERIC, "C");
        std::ifstream in(filePath);
        if (!in.is_open())
        {
            logError("Failed to open Prefab file for instantiating: " + filePath);
            return nullptr;
        }

        json jpref = json::parse(in, nullptr, false);
        if (jpref.is_discarded() || !jpref.is_object())
        {
            logError("Invalid JSON in Prefab file: " + filePath);
            return nullptr;
        }

        json rootJson;
        if (jpref.contains("rootObject") && jpref.at("rootObject").is_object())
            rootJson = jpref.at("rootObject");
        else if (jpref.contains("object") && jpref.at("object").is_object())
            rootJson = jpref.at("object");
        else
            rootJson = jpref;

        EditorWorld &w = activeWorld();
        EditorObject rootObj;
        deserializeObjectFromJson(rootJson, rootObj);

        int oldRootId = rootObj.id;
        int newRootId = w.allocateObjectId();
        rootObj.id = newRootId;
        rootObj.parentId = parentId;

        RoninEngine::Runtime::Vec2 originalPos = rootObj.position;
        rootObj.position = spawnPos;

        // Reset component IDs to 0 so fresh unique IDs will be assigned
        rootObj.sprite.componentId = 0;
        rootObj.camera.componentId = 0;
        rootObj.light.componentId = 0;
        rootObj.terrain.componentId = 0;
        rootObj.particles.componentId = 0;
        rootObj.physics.componentId = 0;
        rootObj.rigidbody2D.componentId = 0;
        rootObj.boxCollider2D.componentId = 0;
        rootObj.multiCollider2D.componentId = 0;
        rootObj.navMeshAgent.componentId = 0;
        rootObj.audio.componentId = 0;
        rootObj.script.componentId = 0;
        ensureComponentIds(rootObj);

        size_t childCount = (jpref.contains("children") && jpref.at("children").is_array()) ? jpref.at("children").size() : 0;
        w.objects.reserve(w.objects.size() + 1 + childCount);
        w.objects.push_back(rootObj);

        // Deserializing children if present
        if (childCount > 0)
        {
            const auto &jchildren = jpref.at("children");
            std::unordered_map<int, int> idMap;
            idMap[oldRootId] = newRootId;

            std::vector<EditorObject> newChildren;
            newChildren.reserve(childCount);
            for (const auto &jc : jchildren)
            {
                if (!jc.is_object())
                    continue;

                EditorObject childObj;
                deserializeObjectFromJson(jc, childObj);

                int oldChildId = childObj.id;
                int newChildId = w.allocateObjectId();
                idMap[oldChildId] = newChildId;

                childObj.id = newChildId;
                childObj.position = spawnPos + (childObj.position - originalPos);

                childObj.sprite.componentId = 0;
                childObj.camera.componentId = 0;
                childObj.light.componentId = 0;
                childObj.terrain.componentId = 0;
                childObj.particles.componentId = 0;
                childObj.physics.componentId = 0;
                childObj.rigidbody2D.componentId = 0;
                childObj.boxCollider2D.componentId = 0;
                childObj.multiCollider2D.componentId = 0;
                childObj.navMeshAgent.componentId = 0;
                childObj.audio.componentId = 0;
                childObj.script.componentId = 0;
                ensureComponentIds(childObj);

                newChildren.push_back(childObj);
            }

            // Remap parentId only for newly instantiated children
            for (auto &child : newChildren)
            {
                if (idMap.count(child.parentId))
                {
                    child.parentId = idMap[child.parentId];
                }
                w.objects.push_back(child);
            }
        }

        isDirty = true;
        selectObject(newRootId);
        logInfo("Instantiated prefab '" + rootObj.name + "' from " + filePath);
        return findObject(newRootId);
    }

    bool EditorState::saveTerrainAsset(const Terrain2DData &terrain, const std::string &filePath)
    {
        std::setlocale(LC_NUMERIC, "C");
        std::filesystem::path p(filePath);
        std::error_code ec;
        if (!std::filesystem::exists(p.parent_path(), ec))
        {
            std::filesystem::create_directories(p.parent_path(), ec);
        }

        std::ofstream out(filePath);
        if (!out.is_open())
        {
            logError("Failed to open file for writing terrain asset: " + filePath);
            return false;
        }

        json jter = json::object();
        jter["format"] = "RoninEngineTerrain";
        jter["terrainVersion"] = 1;
        jter["terrain"] = serializeTerrainToJson(terrain);

        out << jter.dump(2) << "\n";
        out.close();

        // Ensure companion .meta file is generated
        MetaFile mf;
        MetaManager::EnsureMetaFile(filePath, mf);
        AssetDatabase::GetInstance().Refresh();

        logInfo("Saved Terrain asset to " + filePath);
        return true;
    }

    bool EditorState::loadTerrainAsset(Terrain2DData &terrain, const std::string &filePath)
    {
        std::setlocale(LC_NUMERIC, "C");
        std::ifstream in(filePath);
        if (!in.is_open())
        {
            logError("Failed to open Terrain asset for loading: " + filePath);
            return false;
        }

        std::stringstream buffer;
        buffer << in.rdbuf();
        json j = json::parse(buffer.str(), nullptr, false);
        if (j.is_discarded() || !j.is_object())
        {
            logError("Invalid JSON in Terrain asset: " + filePath);
            return false;
        }

        const json &node = j.contains("terrain") ? j.at("terrain") : j;
        deserializeTerrainFromJson(node, terrain);
        terrain.enabled = true;
        isDirty = true;

        logInfo("Loaded Terrain asset from " + filePath);
        return true;
    }

    void EditorState::updateAtlasSlicer()
    {
        slicedUVs.clear();
        int cols = std::max(1, atlasColumns);
        int rows = std::max(1, atlasRows);
        int w = std::max(1, atlasImageWidth);
        int h = std::max(1, atlasImageHeight);

        float cellW = static_cast<float>(w) / static_cast<float>(cols);
        float cellH = static_cast<float>(h) / static_cast<float>(rows);

        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                slicedUVs.emplace_back(
                    (c * cellW) / static_cast<float>(w),
                    (r * cellH) / static_cast<float>(h),
                    cellW / static_cast<float>(w),
                    cellH / static_cast<float>(h));
            }
        }
    }

    void EditorState::updateNavMeshPreview(bool regenerateObstacles, bool generateMaze)
    {
        NavMeshPreviewData &navPreview = activeWorld().navMesh;
        EditorObject *terObj = findActiveTerrainObject();

        int w = (terObj && terObj->terrain.width >= 4) ? terObj->terrain.width : std::max(4, navPreview.width);
        int h = (terObj && terObj->terrain.length >= 4) ? terObj->terrain.length : std::max(4, navPreview.height);
        navPreview.width = w;
        navPreview.height = h;

        if (terObj)
        {
            terObj->terrain.ensureGridInitialized(0);
        }

        if (regenerateObstacles || static_cast<int>(navPreview.lockedCells.size()) != w * h)
        {
            if (terObj)
            {
                navPreview.lockedCells.assign(w * h, false);
                for (int y = 0; y < h; ++y)
                {
                    for (int x = 0; x < w; ++x)
                    {
                        size_t idx = static_cast<size_t>(y * w + x);
                        if (idx < terObj->terrain.cellColliders.size() && terObj->terrain.cellColliders[idx])
                            navPreview.lockedCells[idx] = true;
                    }
                }
            }
            else
            {
                initDefaultNavMesh(navPreview);
                w = navPreview.width;
                h = navPreview.height;
            }
        }

        navPreview.start.x = Math::Clamp(navPreview.start.x, 0, w - 1);
        navPreview.start.y = Math::Clamp(navPreview.start.y, 0, h - 1);
        navPreview.goal.x = Math::Clamp(navPreview.goal.x, 0, w - 1);
        navPreview.goal.y = Math::Clamp(navPreview.goal.y, 0, h - 1);

        AI::NavMesh nav(w, h);
        nav.Clear(true);

        nav.setHeuristic(static_cast<AI::HeuristicMethod>(std::clamp(navPreview.heuristicMethod, 0, 3)));
        nav.setIdentity(static_cast<AI::NavIdentity>(std::clamp(navPreview.identityMethod, 0, 2)));

        if (generateMaze)
        {
            nav.GenerateMaze(static_cast<AI::MazeAlgorithm>(std::clamp(navPreview.mazeAlgorithm, 0, 12)));
            nav.setLock(navPreview.start, false);
            nav.setLock(navPreview.goal, false);
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    bool locked = nav.hasLocked(Vec2Int(x, y));
                    navPreview.lockedCells[y * w + x] = locked;
                    if (terObj && static_cast<size_t>(y * w + x) < terObj->terrain.cellColliders.size())
                    {
                        terObj->terrain.cellColliders[y * w + x] = locked;
                    }
                }
            }
            if (terObj)
                isDirty = true;
        }
        else
        {
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    size_t idx = static_cast<size_t>(y * w + x);
                    bool locked = false;
                    if (terObj)
                    {
                        locked = (idx < terObj->terrain.cellColliders.size()) ? terObj->terrain.cellColliders[idx] : false;
                    }
                    else
                    {
                        locked = (idx < navPreview.lockedCells.size()) ? navPreview.lockedCells[idx] : false;
                    }

                    if ((x == navPreview.start.x && y == navPreview.start.y) ||
                        (x == navPreview.goal.x && y == navPreview.goal.y))
                    {
                        locked = false;
                        if (terObj && idx < terObj->terrain.cellColliders.size())
                            terObj->terrain.cellColliders[idx] = false;
                    }
                    navPreview.lockedCells[idx] = locked;
                    nav.setLock(Vec2Int(x, y), locked);
                }
            }
        }

        AI::NavResultSite result;
        navPreview.pathFound = nav.Find(result, navPreview.start, navPreview.goal);
        navPreview.path = result.connections;
    }

    void EditorState::logInfo(const std::string &msg)
    {
        logs.push_back({currentTimestamp(), "INFO", msg});
    }

    void EditorState::logWarn(const std::string &msg)
    {
        logs.push_back({currentTimestamp(), "WARN", msg});
    }

    void EditorState::logError(const std::string &msg)
    {
        logs.push_back({currentTimestamp(), "ERROR", msg});
    }

    void EditorState::clearLogs()
    {
        logs.clear();
    }

    void EditorState::recordUndo(const std::string &actionName)
    {
        redoStack.clear();
        HistoryRecord rec;
        rec.actionName = actionName;
        rec.worldState = activeWorld();
        rec.selectedObjectId = selectedObjectId;
        rec.selectedObjectIds = selectedObjectIds;
        undoStack.push_back(std::move(rec));
        if (undoStack.size() > kMaxHistoryRecords)
        {
            undoStack.erase(undoStack.begin());
        }
    }

    bool EditorState::undo()
    {
        if (undoStack.empty())
            return false;

        HistoryRecord rec = std::move(undoStack.back());
        undoStack.pop_back();

        HistoryRecord redoRec;
        redoRec.actionName = rec.actionName;
        redoRec.worldState = activeWorld();
        redoRec.selectedObjectId = selectedObjectId;
        redoRec.selectedObjectIds = selectedObjectIds;
        redoStack.push_back(std::move(redoRec));

        activeWorld() = std::move(rec.worldState);
        selectedObjectId = rec.selectedObjectId;
        selectedObjectIds = rec.selectedObjectIds;
        isDirty = true;
        logInfo("Undo: " + rec.actionName);
        return true;
    }

    bool EditorState::redo()
    {
        if (redoStack.empty())
            return false;

        HistoryRecord rec = std::move(redoStack.back());
        redoStack.pop_back();

        HistoryRecord undoRec;
        undoRec.actionName = rec.actionName;
        undoRec.worldState = activeWorld();
        undoRec.selectedObjectId = selectedObjectId;
        undoRec.selectedObjectIds = selectedObjectIds;
        undoStack.push_back(std::move(undoRec));

        activeWorld() = std::move(rec.worldState);
        selectedObjectId = rec.selectedObjectId;
        selectedObjectIds = rec.selectedObjectIds;
        isDirty = true;
        logInfo("Redo: " + rec.actionName);
        return true;
    }

    void EditorState::clearHistory()
    {
        undoStack.clear();
        redoStack.clear();
    }

    std::string EditorState::getEditorVersion()
    {
        return RONIN_EDITOR_VERSION_STRING;
    }

    std::string EditorState::getEngineVersion()
    {
        const char *ver = RoninSimulator::GetVersionString();
        return ver ? std::string(ver) : "unknown";
    }

    std::string EditorState::getLinkedTargets()
    {
        const char *targets = RoninSimulator::GetLinkedTargets();
        return targets ? std::string(targets) : "unknown";
    }
} // namespace RoninEditor
