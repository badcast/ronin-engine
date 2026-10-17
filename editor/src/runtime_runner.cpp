#include "runtime_runner.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using namespace RoninEngine;
using namespace RoninEngine::Runtime;

namespace RoninEditor
{
    static EditorProject g_activeRuntimeProject;
    static int g_activeRuntimeWorldIndex = 0;
    static int g_runtimeMaxFrames = 0;
    static int g_pendingWorldSwitch = -1;
    static std::map<UI::uid, int> g_buttonWorldTargets;

    static void onWorldSwitchButtonClicked(UI::uid btnId)
    {
        auto it = g_buttonWorldTargets.find(btnId);
        if (it != g_buttonWorldTargets.end())
        {
            g_pendingWorldSwitch = it->second;
        }
    }

    struct SpawnedMotionEntry
    {
        GameObjectRef go;
        MotionScriptData script;
        Light *lightComp = nullptr;
    };

    class RuntimeProjectWorld : public World
    {
    public:
        int worldIdx = 0;
        int frameCounter = 0;
        std::vector<SpawnedMotionEntry> motionObjects;

        RuntimeProjectWorld()
            : World(
                  (g_activeRuntimeWorldIndex >= 0 &&
                   g_activeRuntimeWorldIndex < static_cast<int>(g_activeRuntimeProject.worlds.size()))
                      ? g_activeRuntimeProject.worlds[g_activeRuntimeWorldIndex].name
                      : "RuntimeWorld"),
              worldIdx(g_activeRuntimeWorldIndex)
        {
        }

        void OnAwake() override
        {
            g_pendingWorldSwitch = -1;
            g_buttonWorldTargets.clear();
            frameCounter = 0;
            motionObjects.clear();

            if (g_activeRuntimeProject.worlds.empty())
            {
                Camera2DRef cam = Primitive::CreateCamera2D();
                cam->Focus();
                return;
            }

            worldIdx = std::clamp(worldIdx, 0, static_cast<int>(g_activeRuntimeProject.worlds.size()) - 1);
            const EditorWorld &ew = g_activeRuntimeProject.worlds[worldIdx];

            std::map<int, GameObjectRef> idToGameObject;
            Camera2DRef focusedCamera = nullptr;

            // 1. First pass: instantiate all GameObjects in this World
            for (const auto &obj : ew.objects)
            {
                GameObjectRef go = Primitive::CreateEmptyGameObject(obj.name, obj.position, obj.angle);
                go->setTag(obj.tag);
                go->SetLayer(obj.layer);
                go->SetZOrder(obj.zOrder);
                go->SetActive(obj.active);
                idToGameObject[obj.id] = go;
            }

            // 2. Second pass: link parent-child Transform hierarchy and set local transforms
            for (const auto &obj : ew.objects)
            {
                auto it = idToGameObject.find(obj.id);
                if (it == idToGameObject.end())
                    continue;
                GameObjectRef go = it->second;

                if (obj.parentId != -1)
                {
                    auto pit = idToGameObject.find(obj.parentId);
                    if (pit != idToGameObject.end() && pit->second != go)
                    {
                        go->transform()->setParent(pit->second->transform(), false);
                        go->transform()->localPosition(obj.position);
                        go->transform()->localAngle(obj.angle);
                    }
                }
            }

            // 3. Third pass: attach all enabled RoninEngine components
            for (const auto &obj : ew.objects)
            {
                auto it = idToGameObject.find(obj.id);
                if (it == idToGameObject.end())
                    continue;
                GameObjectRef go = it->second;

                // Camera2D
                if (obj.camera.enabled)
                {
                    Camera2DRef cam = go->AddComponent<Camera2D>();
                    cam->backclear = obj.camera.backclear;
                    cam->backcolor = obj.camera.backcolor;
                    Vec2 targetScale = (obj.camera.scale.x != 1.0f || obj.camera.scale.y != 1.0f)
                                           ? obj.camera.scale
                                           : Vec2(obj.camera.zoom, obj.camera.zoom);
                    cam->SetScale(targetScale);
                    if (obj.camera.zoomOut != 100.0f)
                    {
                        cam->SetZoomOut(obj.camera.zoomOut);
                    }
                    cam->distanceEvcall = static_cast<std::uint16_t>(std::max(0, obj.camera.distanceEvcall));
                    cam->visibleBorders = obj.camera.visibleBorders;
                    cam->visibleGrids = obj.camera.visibleGrids;
                    cam->visibleObjects = obj.camera.visibleObjects;
                    cam->visibleNames = obj.camera.visibleNames;
                    if (obj.camera.mainCamera || !focusedCamera)
                    {
                        cam->Focus();
                        focusedCamera = cam;
                    }
                }

                // SpriteRenderer
                if (obj.sprite.enabled)
                {
                    SpriteRendererRef sr = go->AddComponent<SpriteRenderer>();
                    Vec2 finalSize(
                        std::max(0.1f, obj.sprite.size.x * std::fabs(obj.scale.x)),
                        std::max(0.1f, obj.sprite.size.y * std::fabs(obj.scale.y)));

                    SpriteRef spr = nullptr;
                    if (obj.sprite.primitiveShape == 1)
                        spr = Primitive::CreateSpriteCircle(finalSize, 1.0f, obj.sprite.color);
                    else if (obj.sprite.primitiveShape == 2)
                        spr = Primitive::CreateSpriteTriangle(finalSize, 1.0f, obj.sprite.color);
                    else
                        spr = Primitive::CreateSpriteRectangle(finalSize, obj.sprite.color);

                    sr->setSprite(spr);
                    sr->setSize(finalSize);
                    sr->setColor(obj.sprite.color);
                    sr->setRenderType(obj.sprite.renderType == 1 ? SpriteRenderType::Tile : SpriteRenderType::Simple);
                    sr->setRenderOut(obj.sprite.renderOut == 1 ? SpriteRenderOut::Centering : SpriteRenderOut::Origin);
                    sr->setPresentMode(obj.sprite.presentMode == 1 ? SpriteRenderPresentMode::Place : SpriteRenderPresentMode::Fixed);
                    sr->flip = static_cast<SpriteRenderFlip>(std::clamp(obj.sprite.flip, 0, 2));
                }

                // Light / Spotlight
                Light *attachedLight = nullptr;
                if (obj.light.enabled)
                {
                    if (obj.light.lightType == 2)
                    {
                        SpotlightRef sp = go->AddComponent<Spotlight>();
                        sp->intensity = obj.light.intensity;
                        sp->range = obj.light.range;
                        sp->angle = obj.light.angleDegrees;
                        sp->color = obj.light.color;
                        attachedLight = sp.get();
                    }
                    else if (obj.light.lightType == 1)
                    {
                        Ref<DirectLight> dl = go->AddComponent<DirectLight>();
                        dl->intensity = obj.light.intensity;
                        dl->range = obj.light.range;
                        dl->color = obj.light.color;
                        attachedLight = dl.get();
                    }
                    else
                    {
                        Ref<SimpleLight> sl = go->AddComponent<SimpleLight>();
                        sl->intensity = obj.light.intensity;
                        sl->range = obj.light.range;
                        sl->color = obj.light.color;
                        attachedLight = sl.get();
                    }
                }

                // Terrain2D
                if (obj.terrain.enabled)
                {
                    Terrain2DRef ter = go->AddComponent<Terrain2D>();
                    int tw = std::clamp(obj.terrain.width, 1, 128);
                    int tl = std::clamp(obj.terrain.length, 1, 128);
                    float scale = std::max(0.05f, obj.terrain.cellScale);
                    ter->setSize(Vec2(static_cast<float>(tw) * scale, static_cast<float>(tl) * scale));
                    ter->setTextureScale(Vec2(scale, scale));

                    // Surface Painting: Initialize high-resolution terrain surface canvas
                    int surfW = std::clamp(tw * 32, 64, 1024);
                    int surfH = std::clamp(tl * 32, 64, 1024);
                    ter->createSurface(surfW, surfH, obj.terrain.tileColor);

                    // Paint per-cell materials
                    for (int r = 0; r < tl; ++r)
                    {
                        for (int c = 0; c < tw; ++c)
                        {
                            size_t idx = static_cast<size_t>(r * tw + c);
                            int mat = (idx < obj.terrain.cellMaterials.size()) ? obj.terrain.cellMaterials[idx] : 0;
                            Color cCol = (idx < obj.terrain.cellColors.size())
                                             ? obj.terrain.cellColors[idx]
                                             : EditorState::getTerrainMaterialBaseColor(mat, obj.terrain.tileColor);
                            ter->paintRect(Rectf(c * scale, r * scale, scale, scale), cCol, false);

                            // Set colliders if cell marked as obstacle
                            bool hasCol = (idx < obj.terrain.cellColliders.size()) ? obj.terrain.cellColliders[idx] : false;
                            if (hasCol)
                            {
                                ter->setCollider(Rectf(obj.position.x + c * scale, obj.position.y + r * scale, scale, scale), true);
                            }
                        }
                    }

                    // Surface Brush Paint Strokes (freehand painting layers)
                    for (const auto &st : obj.terrain.paintStrokes)
                    {
                        Vec2 strokeLocal(st.uvPos.x * tw * scale, st.uvPos.y * tl * scale);
                        float strokeRadWorld = st.radiusCells * scale;
                        Color strokeCol = st.color;
                        strokeCol.a = static_cast<uint8_t>(std::round(st.color.a * std::clamp(st.opacity, 0.05f, 1.0f)));
                        ter->paintBrush(strokeLocal, strokeRadWorld, strokeCol, st.hardness);
                    }

                    // Border colliders
                    if (obj.terrain.generateBorderColliders)
                    {
                        ter->setCollider(Rectf(obj.position.x, obj.position.y, tw * scale, 0.5f), true);
                        ter->setCollider(Rectf(obj.position.x, obj.position.y + tl * scale - 0.5f, tw * scale, 0.5f), true);
                        ter->setCollider(Rectf(obj.position.x, obj.position.y, 0.5f, tl * scale), true);
                        ter->setCollider(Rectf(obj.position.x + tw * scale - 0.5f, obj.position.y, 0.5f, tl * scale), true);
                    }
                }

                // ParticleSystem
                if (obj.particles.enabled)
                {
                    ParticleSystemRef ps = go->AddComponent<ParticleSystem>();
                    ps->emit = obj.particles.emit;
                    ps->loop = obj.particles.loop;
                    ps->rotate = obj.particles.rotate;
                    ps->worldSpace = obj.particles.worldSpace;
                    ps->randomDirection = obj.particles.randomDirection;
                    ps->interval = std::max(0.02f, obj.particles.interval);
                    ps->speed = obj.particles.speed;
                    ps->rotatePerFrame = obj.particles.rotatePerFrame;
                    ps->direction = obj.particles.direction;
                    if (obj.particles.maxLimit > 0)
                        ps->setLimit(obj.particles.maxLimit);
                    else
                        ps->setLimitInfinitely();

                    SpriteRef partSpr = nullptr;
                    if (obj.particles.primitiveSource == 0)
                        partSpr = Primitive::CreateSpriteCircle(Vec2::one, 1.0f, Color::white);
                    else if (obj.particles.primitiveSource == 2)
                        partSpr = Primitive::CreateSpriteTriangle(Vec2::one, 1.0f, Color::white);
                    else
                        partSpr = Primitive::CreateSpriteRectangle(Vec2::one, Color::white);

                    ps->setSource(partSpr);
                    ps->setInterpolates(std::max(0.2f, obj.particles.duration));
                    ps->setColors(obj.particles.startColor, obj.particles.endColor);
                    ps->setSizes(obj.particles.startSize, obj.particles.endSize);
                }

                // MoveController2D
                if (obj.physics.moveControllerEnabled)
                {
                    MoveController2DRef mc = go->AddComponent<MoveController2D>();
                    mc->speed = obj.physics.moveSpeed;
                    mc->playerCamera = focusedCamera;
                }

                // AudioSource
                if (obj.audio.enabled)
                {
                    AudioSourceRef as = go->AddComponent<AudioSource>();
                    as->setVolume(obj.audio.volume);
                }

                // MotionScript
                if (obj.script.enabled)
                {
                    motionObjects.push_back({go, obj.script, attachedLight});
                }
            }

            if (!focusedCamera)
            {
                Camera2DRef fallback = Primitive::CreateCamera2D();
                fallback->Focus();
            }

            // 4. Build World::GetGUI() widgets
            UI::GUI *gui = GetGUI();
            if (gui)
            {
                for (const auto &ui : ew.guiElements)
                {
                    if (!ui.visible)
                        continue;

                    if (ui.type == 0)
                    {
                        gui->PushLabel(ui.text, ui.rect, std::max(10, ui.fontSize));
                    }
                    else if (ui.type == 1)
                    {
                        std::string targetWorldName = ui.targetWorldOnClick;
                        int targetIdx = -1;
                        if (!targetWorldName.empty())
                        {
                            for (size_t i = 0; i < g_activeRuntimeProject.worlds.size(); ++i)
                            {
                                if (g_activeRuntimeProject.worlds[i].name == targetWorldName)
                                {
                                    targetIdx = static_cast<int>(i);
                                    break;
                                }
                            }
                        }

                        if (targetIdx >= 0)
                        {
                            UI::uid btnId = gui->PushButton(ui.text, ui.rect, onWorldSwitchButtonClicked);
                            g_buttonWorldTargets[btnId] = targetIdx;
                        }
                        else
                        {
                            gui->PushButton(ui.text, ui.rect);
                        }
                    }
                    else if (ui.type == 2)
                    {
                        gui->PushCheckBox(ui.boolValue, ui.text, ui.rect);
                    }
                    else if (ui.type == 3)
                    {
                        gui->PushSlider(ui.floatValue, ui.minValue, ui.maxValue, ui.rect);
                    }
                    else if (ui.type == 4)
                    {
                        gui->PushTextEdit(ui.text, ui.rect);
                    }
                    else if (ui.type == 6)
                    {
                        std::vector<std::string> items;
                        std::istringstream iss(ui.itemsCsv);
                        std::string item;
                        while (std::getline(iss, item, ','))
                        {
                            items.push_back(item);
                        }
                        if (items.empty())
                            items.push_back(ui.text);
                        gui->PushDropDown(items, std::clamp(ui.selectedIndex, 0, static_cast<int>(items.size()) - 1), ui.rect);
                    }
                }

                // Runtime top-right World Switcher bar
                int rightX = std::max(300, g_activeRuntimeProject.resolutionWidth - 230);
                gui->PushLabel("Project Worlds:", Rect(rightX, 14, 210, 22), 12);
                for (size_t i = 0; i < g_activeRuntimeProject.worlds.size(); ++i)
                {
                    int idx = static_cast<int>(i);
                    std::string btnTitle = (idx == worldIdx ? "* " : "  ") + g_activeRuntimeProject.worlds[i].name;
                    UI::uid btnId = gui->PushButton(btnTitle, Rect(rightX, 40 + idx * 34, 210, 28), onWorldSwitchButtonClicked);
                    g_buttonWorldTargets[btnId] = idx;
                }
            }
        }

        void OnUpdate() override
        {
            frameCounter++;
            float dt = Time::deltaTime();
            float t = Time::time();

            for (auto &entry : motionObjects)
            {
                if (!entry.go || !entry.go->isActive())
                    continue;

                if (entry.script.angularVelocity != 0.0f)
                {
                    entry.go->transform()->angle(entry.go->transform()->angle() + entry.script.angularVelocity * dt);
                }

                if (entry.script.linearVelocity.x != 0.0f || entry.script.linearVelocity.y != 0.0f)
                {
                    float dir = Math::Cos(t * 1.2f);
                    Vec2 p = entry.go->transform()->localPosition();
                    p.x += entry.script.linearVelocity.x * dir * dt;
                    p.y += entry.script.linearVelocity.y * dir * dt;
                    entry.go->transform()->localPosition(p);
                }

                if (entry.script.pulseSpeed != 0.0f && entry.script.pulseAmplitude != 0.0f && entry.lightComp)
                {
                    float wave = Math::Sin(t * entry.script.pulseSpeed);
                    entry.lightComp->intensity = std::max(0.2f, 1.0f + wave * entry.script.pulseAmplitude);
                }
            }

            if (g_pendingWorldSwitch >= 0 &&
                g_pendingWorldSwitch < static_cast<int>(g_activeRuntimeProject.worlds.size()) &&
                g_pendingWorldSwitch != worldIdx)
            {
                g_activeRuntimeWorldIndex = g_pendingWorldSwitch;
                g_pendingWorldSwitch = -1;
                RoninSimulator::LoadWorld<RuntimeProjectWorld>();
                return;
            }

            if (g_runtimeMaxFrames > 0 && frameCounter >= g_runtimeMaxFrames)
            {
                RoninSimulator::RequestQuit();
            }
        }

        void OnUnloading() override
        {
            motionObjects.clear();
            g_buttonWorldTargets.clear();
        }
    };

    int RuntimeRunner::runProject(const EditorProject &project, int initialWorldIndex, int maxFrames)
    {
        if (project.worlds.empty())
        {
            std::cerr << "[RuntimeRunner] Project contains no Worlds to run." << std::endl;
            return 1;
        }

        g_activeRuntimeProject = project;
        if (initialWorldIndex >= 0 && initialWorldIndex < static_cast<int>(project.worlds.size()))
            g_activeRuntimeWorldIndex = initialWorldIndex;
        else
            g_activeRuntimeWorldIndex = std::clamp(project.startupWorldIndex, 0, static_cast<int>(project.worlds.size()) - 1);

        g_runtimeMaxFrames = maxFrames;
        g_pendingWorldSwitch = -1;

        RoninSimulator::Init(InitializeFlags::Graphics);
        RoninSimulator::Show(
            Resolution(
                std::max(320, project.resolutionWidth),
                std::max(240, project.resolutionHeight),
                std::max(30, project.refreshRate)),
            project.fullscreen);

        RoninSettings settings = RoninSimulator::GetSettings();
        settings.renderBackend = (project.renderBackend == 1)
                                     ? RenderDriverInfo::RenderBackend::CPU
                                     : RenderDriverInfo::RenderBackend::GPU;
        settings.textureQuality = static_cast<std::uint8_t>(std::clamp(project.textureQuality, 0, 1));
        settings.verticalSync = project.verticalSync;
        settings.brightness = std::clamp(project.brightness, 0.2f, 1.0f);
        settings.windowOpacity = std::clamp(project.windowOpacity, 0.2f, 1.0f);
        settings.Apply();

        RoninSimulator::SetDebugMode(project.debugMode);

        RuntimeProjectWorld *loaded = RoninSimulator::LoadWorld<RuntimeProjectWorld>();
        if (!loaded)
        {
            std::cerr << "[RuntimeRunner] Failed to load RuntimeProjectWorld." << std::endl;
            RoninSimulator::Finalize();
            return 1;
        }

        RoninSimulator::Simulate();
        RoninSimulator::Finalize();
        return 0;
    }

    int RuntimeRunner::runProjectFromFile(const std::string &projectFilePath, int initialWorldIndex, int maxFrames)
    {
        EditorState state;
        if (!state.loadProjectFromFile(projectFilePath))
        {
            std::cerr << "[RuntimeRunner] Failed to load project file: " << projectFilePath << std::endl;
            return 1;
        }
        return runProject(state.project, initialWorldIndex, maxFrames);
    }
} // namespace RoninEditor
