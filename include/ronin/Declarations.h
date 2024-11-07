#pragma once

#include <cstdint>

#ifndef RONIN_COMPILLING
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
        struct Color;

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

    } // namespace Runtime
} // namespace RoninEngine
