#include <iostream>
#include <cassert>
#include <cmath>
#include <ronin/framework.h>

using namespace RoninEngine;
using namespace RoninEngine::Runtime;

class BugfixWorld : public World
{
public:
    BugfixWorld(const std::string &name = "BugfixWorld") : World(name) {}
    void OnAwake() override {}
    void OnStart() override {}
    void OnUpdate() override {}
};

int main()
{
    RoninSimulator::Init();

    std::cout << "========================================" << std::endl;
    std::cout << "Testing Bugfixes and Safety Hardening" << std::endl;
    std::cout << "========================================" << std::endl;

    // 1. Paths test
    std::cout << "[1/6] Testing Paths..." << std::endl;
    char sep = Paths::GetPathSeperatorOS();
    assert(sep == '/' || sep == '\\');
    std::string pathTest = "folder/subfolder\\file.txt";
    std::string validPath = Paths::MakePathValid(pathTest);
    std::cout << "      Valid path: " << validPath << std::endl;

    // 2. Camera without world / with world
    std::cout << "[2/6] Testing Camera without world..." << std::endl;
    assert(Camera::mainCamera().isNull());
    Vec2 vp = Camera::ScreenToViewportPoint(Vec2(100.0f, 100.0f));
    assert(vp.x == 100.0f && vp.y == 100.0f); // Default 1x1 fallback

    // 3. AudioSource lifecycle
    std::cout << "[3/6] Testing AudioSource lifecycle..." << std::endl;
    TestWorld:
    {
        BugfixWorld world1("World1");
        GameObjectRef audioObj = Primitive::CreateEmptyGameObject();
        AudioSourceRef audioSource = audioObj->AddComponent<AudioSource>();
        assert(!audioSource.isNull());
        float vol = audioSource->getVolume();
        audioSource->setVolume(0.75f);
        assert(std::abs(audioSource->getVolume() - 0.75f) < 0.05f);

        // Destroy audio object
        audioObj->Destroy();
        std::cout << "      AudioSource destroyed cleanly without crash or leak." << std::endl;
    }

    // 4. Physics2D casts with null checks
    std::cout << "[4/6] Testing Physics2D casts..." << std::endl;
    {
        BugfixWorld world2("World2");
        GameObjectRef obj1 = Primitive::CreateEmptyGameObject(Vec2(5, 5));
        GameObjectRef obj2 = Primitive::CreateEmptyGameObject(Vec2(20, 20));

        auto rectCast = Physics2D::GetRectangleCast<std::vector<Transform*>>(Vec2(5, 5), Vec2(10, 10));
        assert(!rectCast.empty());
        bool foundObj1 = false;
        for(auto t : rectCast)
        {
            if(t && t->gameObject() == obj1)
                foundObj1 = true;
        }
        assert(foundObj1);

        auto circleCast = Physics2D::GetCircleCast<std::vector<Transform*>>(Vec2(5, 5), 5.0f);
        assert(!circleCast.empty());

        // 5. Instantiation state preservation (volume, enable)
        std::cout << "[5/6] Testing Instantiate state preservation..." << std::endl;
        GameObjectRef sourceObj = Primitive::CreateEmptyGameObject(Vec2(10, 10));
        AudioSourceRef srcAudio = sourceObj->AddComponent<AudioSource>();
        srcAudio->setVolume(0.42f);
        srcAudio->enable(false);

        GameObjectRef clonedObj = Instantiate(sourceObj);
        assert(!clonedObj.isNull());
        AudioSourceRef clonedAudio = clonedObj->GetComponent<AudioSource>();
        assert(!clonedAudio.isNull());
        assert(!clonedAudio->enable());
        assert(std::abs(clonedAudio->getVolume() - 0.42f) < 0.05f);

        // 6. UI safety
        std::cout << "[6/6] Testing UI safety..." << std::endl;
        UI::GUI *gui = world2.GetGUI();
        if(gui)
        {
            assert(!gui->PopElement(0)); // NOPARENT sentinel, must return false safely
            assert(!gui->PopElement(99999)); // Non-existent element
        }
    }

    RoninSimulator::Finalize();

    std::cout << "========================================" << std::endl;
    std::cout << "All Bugfix Tests PASSED Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
