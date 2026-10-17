#include <iostream>
#include <cassert>
#include <ronin/framework.h>

using namespace RoninEngine;
using namespace RoninEngine::Runtime;
using namespace RoninEngine::AI;
using namespace RoninEngine::UI;

class TestWorld : public World
{
public:
    TestWorld(const std::string &name = "TestWorld") : World(name) {}
    void OnAwake() override {}
    void OnStart() override {}
    void OnUpdate() override {}
};

class DummyScript : public Behaviour
{
public:
    int updateTicks = 0;
    DummyScript() : Behaviour("DummyScript") {}
    void OnUpdate() override
    {
        updateTicks++;
    }
};

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "Starting Comprehensive Ronin Engine Test" << std::endl;
    std::cout << "========================================" << std::endl;

    // 1. Version & Performances
    VersionInfo vInfo = RoninSimulator::GetVersion();
    std::cout << "[1/8] Engine Version: " << (int)vInfo.Engine_Version.major << "." 
              << (int)vInfo.Engine_Version.minor << "." << (int)vInfo.Engine_Version.patch << std::endl;
    std::cout << "      Version String: " << RoninSimulator::GetVersionString() << std::endl;
    assert(std::string(RoninSimulator::GetVersionString()) == "0.0.1-alpha");
    std::cout << "      Memory: " << Perfomances::GetMemoryUsed() << " bytes" << std::endl;
    std::cout << "      CPU Usage: " << Perfomances::GetCPUUsage() << "%" << std::endl;

    // 2. Math, Vec2 & Physics2D SAT Collision
    std::cout << "[2/8] Testing Math & SAT Collision..." << std::endl;
    Vec2 v1(3.0f, 4.0f);
    assert(Math::Abs(v1.magnitude() - 5.0f) < 0.001f);
    assert(Math::Abs(v1.normalized().magnitude() - 1.0f) < 0.001f);

    Rectf rect1(0, 0, 10, 10);
    Rectf rect2(5, 5, 10, 10);
    Rectf rect3(30, 30, 10, 10);
    assert(Physics2D::CheckCollision(rect1, 0.0f, rect2, 0.0f) == true);
    assert(Physics2D::CheckCollision(rect1, 0.0f, rect3, 0.0f) == false);
    // SAT Rotated collision test (45 degrees)
    assert(Physics2D::CheckCollision(rect1, 0.0f, rect2, Math::deg2rad * 45.0f) == true);

    // 3. NavMesh & AI Search
    std::cout << "[3/8] Testing NavMesh & Free Space Search..." << std::endl;
    NavMesh nav(30, 30);
    assert(nav.getWidth() == 30);
    assert(nav.getHeight() == 30);
    nav.setLock(Vec2Int(10, 10), true);
    assert(nav.hasLocked(Vec2Int(10, 10)) == true);
    assert(nav.hasLocked(Vec2Int(10, 11)) == false);

    Vec2Int freePt = nav.FindFreeSpace(Vec2Int(10, 10), 5);
    assert(nav.hasLocked(freePt) == false);

    // 4. World, Hierarchy & ObjectMoveTo
    std::cout << "[4/8] Testing World, Hierarchy & ObjectMoveTo..." << std::endl;
    TestWorld worldA("WorldA");
    TestWorld worldB("WorldB");
    GameObjectRef parentObj = Primitive::CreateEmptyGameObject("ParentObj");
    GameObjectRef childObj = Primitive::CreateEmptyGameObject("ChildObj");
    childObj->transform()->setParent(parentObj->transform());
    assert(childObj->transform()->parent() == parentObj->transform());

    Ref<DummyScript> script = childObj->AddComponent<DummyScript>();
    assert(!script.isNull());

    // Move childObj to worldB
    worldA.ObjectMoveTo(childObj, &worldB);
    assert(childObj->transform()->parent() != parentObj->transform());
    std::cout << "      Object moved between worlds successfully." << std::endl;

    // 5. Atlas & Slicing
    std::cout << "[5/8] Testing Atlas & Grid Slicing..." << std::endl;
    Atlas atlas;
    Rect rSheet{0, 0, 64, 64};
    atlas.AddSprite("sheet", rSheet);
    assert(atlas.GetCount() == 1);
    std::vector<SpriteRef> gridSlices = atlas.SliceGrid(16, 16);
    assert(gridSlices.size() == 16);
    assert(atlas.GetCount() == 17);
    SpriteRef slice0 = atlas[std::string("tile_0")];
    assert(!slice0.isNull());
    assert(slice0->width() == 16);
    assert(slice0->height() == 16);

    // 6. Terrain2D & Colliders
    std::cout << "[6/8] Testing Terrain2D Mesh & Colliders..." << std::endl;
    GameObjectRef terrainObj = Primitive::CreateEmptyGameObject("TerrainObj");
    Ref<Terrain2D> terrain = terrainObj->AddComponent<Terrain2D>();
    assert(!terrain.isNull());
    terrain->setMeshTile(0, 0, slice0);
    terrain->setMeshTile(1, 0, slice0);
    terrain->setCollider(Rectf(0, 0, 4, 4), true);
    assert(terrain->is_collider(Vec2(2, 2)) == true);
    assert(terrain->is_collider(Vec2(25, 25)) == false);

    // Circle collider test
    terrain->setColliderCircle(Vec2(-2, -2), 1.5f, true);
    assert(terrain->is_collider(Vec2(-2, -2)) == true);
    assert(terrain->is_collider(Vec2(-2, 0)) == false);

    terrain->clearColliders();
    assert(terrain->is_collider(Vec2(2, 2)) == false);

    // 7. Light Components
    std::cout << "[7/8] Testing 2D Lights (SimpleLight, Spotlight, DirectLight)..." << std::endl;
    GameObjectRef lightObj = Primitive::CreateEmptyGameObject("Lights");
    Ref<SimpleLight> sLight = lightObj->AddComponent<SimpleLight>();
    sLight->range = 50.0f;
    sLight->intensity = 0.9f;
    sLight->color = Color::yellow;
    assert(sLight->range == 50.0f);
    assert(sLight->intensity == 0.9f);

    Ref<Spotlight> spLight = lightObj->AddComponent<Spotlight>();
    spLight->angle = 60.0f;
    spLight->range = 40.0f;
    assert(spLight->angle == 60.0f);

    Ref<DirectLight> dLight = lightObj->AddComponent<DirectLight>();
    dLight->intensity = 0.75f;
    assert(dLight->intensity == 0.75f);

    // 8. UI System
    std::cout << "[8/8] Testing GUI & Interactive Controls..." << std::endl;
    GUI *gui = worldA.GetGUI();
    assert(gui != nullptr);

    uid grp = gui->PushGroup(Rect(0, 0, 400, 400));
    assert(gui->IsGroup(grp) == true);

    uid btn = gui->PushButton("Start Game", Vec2Int(20, 20), nullptr, grp);
    assert(gui->ElementContains(btn) == true);
    assert(gui->IsGroup(btn) == false);

    uid lbl = gui->PushLabel("Player Status", Vec2Int(20, 60), 13, grp);
    assert(gui->ElementGetText(lbl) == "Player Status");
    gui->ElementSetText(lbl, "Updated Status");
    assert(gui->ElementGetText(lbl) == "Updated Status");

    uid sld = gui->PushSlider(0.5f, 0.0f, 100.0f, Vec2Int(20, 100), nullptr, grp);
    assert(Math::Abs(gui->SliderGetValue(sld) - 0.5f) < 0.001f);
    gui->SliderSetValue(sld, 75.0f);
    assert(Math::Abs(gui->SliderGetValue(sld) - 75.0f) < 0.001f);

    uid chk = gui->PushCheckBox(true, "Mute Music", Vec2Int(20, 140), nullptr, grp);
    assert(gui->CheckBoxGetValue(chk) == true);
    gui->CheckBoxSetValue(chk, false);
    assert(gui->CheckBoxGetValue(chk) == false);

    bool popped = gui->PopElement(btn);
    assert(popped == true);
    assert(gui->ElementContains(btn) == false);

    std::cout << "========================================" << std::endl;
    std::cout << "All Comprehensive Unit Tests PASSED! [8/8]" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
