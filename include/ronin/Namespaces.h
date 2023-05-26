#pragma once

// Pre declarations
namespace RoninEngine
{
    class Application;

    namespace UI
    {
        struct UIElement;
        class GUI;

        // This is control (GUI, UI) identifier type
        typedef std::uint8_t uid;

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

        struct Rendering;

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
        class Renderer;
        class Component;
        class Behaviour;
        class Physics2D;

        // factory
        class GameObject;
        class Texture;
        class Atlas;
        class Sprite;
        struct Color;

        //derived component
        class Transform;
        class Light;
        class Spotlight;
        class SpriteRenderer;
        class Terrain2D;
        class Camera;
        class Camera2D;
        class MoveController2D;

    } // namespace Runtime
} // namespace RoninEngine
