#pragma once

// Pre declarations
namespace RoninEngine
{
    class Application;

    namespace UI
    {
        struct UIElement;
        class GUI;
    } // namespace UI

    namespace AIPathFinder
    {
        class Neuron;
        class NavMesh;
        class NavContainer;
    } // namespace AIPathFinder

    namespace Runtime
    {
        class Level;

        struct Vec2;

        struct Vec2Int;

        struct Render_info;

        template <typename T>
        class xRect;

        typedef xRect<int> Rect;
        typedef xRect<float> Rectf_t;

        // core
        class ResourceManager;
        class TimeEngine;

        // base
        class Math;
        class Object;
        class Component;
        class Physics2D;

        // factory
        class Transform;
        class Camera;
        class Camera2D;
        class Texture;
        class Atlas;
        class Sprite;
        class Behaviour;
        class Terrain2D;
        class SpriteRenderer;
        class Spotlight;
        class GameObject;
        class Light;
        class Renderer;
        struct Color;
    } // namespace Runtime
} // namespace RoninEngine
