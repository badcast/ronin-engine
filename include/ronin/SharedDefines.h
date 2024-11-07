#ifndef _RONIN_SHARED_DEFINES_H_
#define _RONIN_SHARED_DEFINES_H_ 1

#include "Declarations.h"
#include "SharedPointer.h"

namespace RoninEngine::Runtime
{
    // Use Bushido<T> to convert any pointer/values on SharedRoninObject<T>

    using ObjectRef = Bushido<Object>;

    using ComponentRef = Bushido<Component>;
    using RendererRef = Bushido<Renderer>;
    using TransformRef = Bushido<Transform>;
    using BehaviourRef = Bushido<Behaviour>;
    using CollisionRef = Bushido<Collision>;
    using LightRef = Bushido<Light>;
    using SpotlightRef = Bushido<Spotlight>;
    using SpriteRendererRef = Bushido<SpriteRenderer>;
    using Terrain2DRef = Bushido<Terrain2D>;
    using CameraRef = Bushido<Camera>;
    using Camera2DRef = Bushido<Camera2D>;
    using MoveController2DRef = Bushido<MoveController2D>;
    using AudioSourceRef = Bushido<AudioSource>;
    using ParticleSystemReSf = Bushido<ParticleSystem>;

    using GameObjectRef = Bushido<GameObject>;

    using SpriteRef = Bushido<Sprite>;
    using AtlasRef = Bushido<Atlas>;
}

#endif
