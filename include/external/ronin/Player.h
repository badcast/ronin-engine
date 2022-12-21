#pragma once
#include "begin.h"

namespace RoninEngine::Runtime {
class SHARK Player : public Behaviour {
   public:
    Camera2D *playerCamera;
    SpriteRenderer *spriteRenderer;
    Spotlight *spotlight;
    Player() : Player(typeid(*this).name()) {}
    Player(const std::string &name) : Behaviour(name) {}
    float speed;

    void OnAwake();
    void OnStart();
    void OnUpdate();
    void OnLateUpdate();
    void OnDestroy();
};
}  // namespace RoninEngine::Runtime
