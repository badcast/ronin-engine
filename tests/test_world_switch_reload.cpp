#include <iostream>
#include <cassert>
#include <ronin/framework.h>

using namespace RoninEngine;
using namespace RoninEngine::Runtime;

static int worldA_awake = 0;
static int worldA_start = 0;
static int worldA_update = 0;
static int worldA_unload = 0;

static int worldB_awake = 0;
static int worldB_start = 0;
static int worldB_update = 0;
static int worldB_unload = 0;

class WorldB;

class WorldA : public World
{
public:
    WorldA() : World("WorldA") {}

    void OnAwake() override
    {
        worldA_awake++;
        std::cout << "  [WorldA] OnAwake called (" << worldA_awake << ")" << std::endl;

        // Create hierarchical GameObjects
        Camera2DRef cam = Primitive::CreateCamera2D();
        GameObjectRef parent = Primitive::CreateEmptyGameObject();
        parent->name("Parent_A");
        for(int i = 0; i < 10; ++i)
        {
            GameObjectRef child = Primitive::CreateEmptyGameObject();
            child->name("Child_A_" + std::to_string(i));
            child->transform()->setParent(parent->transform());
            for(int j = 0; j < 3; ++j)
            {
                GameObjectRef grandchild = Primitive::CreateEmptyGameObject();
                grandchild->name("Grandchild_A_" + std::to_string(i) + "_" + std::to_string(j));
                grandchild->transform()->setParent(child->transform());
            }
        }
    }

    void OnStart() override
    {
        worldA_start++;
        std::cout << "  [WorldA] OnStart called (" << worldA_start << ")" << std::endl;
    }

    void OnUpdate() override;

    void OnUnloading() override
    {
        worldA_unload++;
        std::cout << "  [WorldA] OnUnloading called (" << worldA_unload << ")" << std::endl;
    }
};

class WorldB : public World
{
public:
    WorldB() : World("WorldB") {}

    void OnAwake() override
    {
        worldB_awake++;
        std::cout << "  [WorldB] OnAwake called (" << worldB_awake << ")" << std::endl;

        Camera2DRef cam = Primitive::CreateCamera2D();
        GameObjectRef go = Primitive::CreateEmptyGameObject();
        go->name("WorldB_Object");
    }

    void OnStart() override
    {
        worldB_start++;
        std::cout << "  [WorldB] OnStart called (" << worldB_start << ")" << std::endl;
    }

    void OnUpdate() override
    {
        worldB_update++;
        std::cout << "  [WorldB] OnUpdate frame " << worldB_update << std::endl;

        if(worldB_awake == 1 && worldB_update == 2)
        {
            std::cout << "  [WorldB] Requesting ReloadWorld()..." << std::endl;
            bool reloaded = RoninSimulator::ReloadWorld();
            std::cout << "  [WorldB] ReloadWorld() queued = " << (reloaded ? "SUCCESS" : "FAILED") << std::endl;
            assert(reloaded);
        }
        else if(worldB_awake >= 2 && worldB_update >= 4)
        {
            std::cout << "  [WorldB] Reloaded world running! Now Requesting Quit..." << std::endl;
            RoninSimulator::RequestQuit();
        }
    }

    void OnUnloading() override
    {
        worldB_unload++;
        std::cout << "  [WorldB] OnUnloading called (" << worldB_unload << ")" << std::endl;
    }
};

void WorldA::OnUpdate()
{
    worldA_update++;
    std::cout << "  [WorldA] OnUpdate frame " << worldA_update << " -> Requesting LoadWorld<WorldB>()" << std::endl;

    // Switch to WorldB!
    bool switched = (RoninSimulator::LoadWorld<WorldB>() != nullptr);
    std::cout << "  [WorldA] LoadWorld<WorldB>() result = " << (switched ? "SUCCESS" : "FAILED") << std::endl;
    assert(switched);
}

int main()
{
    std::cout << "=== RUNNING WORLD SWITCH & RELOAD INTEGRATION TEST ===" << std::endl;

    RoninSimulator::Init();
    RoninSimulator::Show(Resolution{640, 480, 60});

    // 1. Initial LoadWorld before Simulate
    WorldA *a = RoninSimulator::LoadWorld<WorldA>();
    assert(a != nullptr);
    std::cout << "[PASS] Step 1: Loaded WorldA initially." << std::endl;

    // 2. Start simulation loop
    std::cout << "[INFO] Entering Simulate()..." << std::endl;
    RoninSimulator::Simulate();
    std::cout << "[INFO] Exited Simulate() cleanly." << std::endl;

    // 3. Finalize
    RoninSimulator::Finalize();
    std::cout << "[PASS] Finalize completed cleanly." << std::endl;

    // 4. Verify lifecycles
    std::cout << "Verification:" << std::endl;
    std::cout << "  WorldA: awake=" << worldA_awake << ", start=" << worldA_start 
              << ", update=" << worldA_update << ", unload=" << worldA_unload << std::endl;
    std::cout << "  WorldB: awake=" << worldB_awake << ", start=" << worldB_start 
              << ", update=" << worldB_update << ", unload=" << worldB_unload << std::endl;

    assert(worldA_awake == 1);
    assert(worldA_start == 1);
    assert(worldA_update >= 1);
    assert(worldA_unload == 1);

    assert(worldB_awake == 2); // initial awake + reloaded awake
    assert(worldB_start == 2); // initial start + reloaded start
    assert(worldB_update >= 3);
    assert(worldB_unload >= 2); // reloaded unload + final quit unload

    std::cout << "=== ALL WORLD SWITCH & RELOAD TESTS PASSED PERFECTLY! ===" << std::endl;
    return 0;
}
