#ifndef _RONIN_DECLARATIONS_H_
#define _RONIN_DECLARATIONS_H_ 1

#if !RONIN_COMPILLING
typedef struct SDL_Surface Image;
typedef struct SDL_Texture native_texture_t;
typedef struct SDL_Cursor Cursor;

struct native_color_t
{
    std::uint8_t r, g, b, a;
};

struct native_rect_t
{
    int x, y, w, h;
};

struct native_frect_t
{
    float x, y, w, h;
};

#endif

// Pre declarations
namespace RoninEngine
{
    class RoninSimulator;

    namespace UI
    {
        class GUI;

        // This is control (GUI, UI) identifier type
        typedef std::uint32_t uid;

    } // namespace UI

    namespace AI
    {
        class Neuron;
        class NavMesh;
        class Sheduller;
    } // namespace AI

    namespace Runtime
    {

        struct Vec2;

        struct Vec2Int;

        struct Rendering;

        struct AudioClip;

        struct MusicClip;

        template <typename T>
        class TRect;

        typedef TRect<int> Rect;
        typedef TRect<float> Rectf;

        // core
        class World;
        class Resources;
        class Time;
        struct Color;

        // base
        class Math;
        class Object;
        class Renderer;
        class Component;
        class Behaviour;
        class Physics2D;
        class Collision;

        // factory
        class GameObject;
        class Atlas;
        class Sprite;

        // derived component
        class Transform;
        class Light;
        class Spotlight;
        class SpriteRenderer;
        class Terrain2D;
        class Camera;
        class Camera2D;
        class MoveController2D;
        class AudioSource;
        class ParticleSystem;
        class Asset;

        class RoninPointer;

        template <typename T>
        class Ref;

        using ObjectRef = Ref<Object>;
        using ComponentRef = Ref<Component>;
        using RendererRef = Ref<Renderer>;
        using TransformRef = Ref<Transform>;
        using BehaviourRef = Ref<Behaviour>;
        using CollisionRef = Ref<Collision>;
        using LightRef = Ref<Light>;
        using SpotlightRef = Ref<Spotlight>;
        using SpriteRendererRef = Ref<SpriteRenderer>;
        using Terrain2DRef = Ref<Terrain2D>;
        using CameraRef = Ref<Camera>;
        using Camera2DRef = Ref<Camera2D>;
        using MoveController2DRef = Ref<MoveController2D>;
        using AudioSourceRef = Ref<AudioSource>;
        using ParticleSystemRef = Ref<ParticleSystem>;
        using GameObjectRef = Ref<GameObject>;
        using SpriteRef = Ref<Sprite>;
        using AtlasRef = Ref<Atlas>;
        using AssetRef = Ref<Asset>;
    } // namespace Runtime
} // namespace RoninEngine

#endif
