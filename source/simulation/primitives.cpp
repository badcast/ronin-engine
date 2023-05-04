#include "ronin.h"

using namespace RoninEngine;
namespace RoninEngine::Runtime
{

    GameObject* Primitive::CreateEmptyGameObject(Vec2 position) { return CreateGameObject(); }

    Camera2D* Primitive::CreateCamera2D(Vec2 position)
    {
        return CreateEmptyGameObject(position)->addComponent<Camera2D>();
    }
}
