#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "Color.h"
#include "Static.h"
#include "Vec2.h"
#include "Paths.h"

// Pre declarations
namespace RoninEngine {
	class Application;
	class Level;

	namespace UI {
		struct RenderData;
		class GUI;
	}  // namespace UI

	namespace AIPathFinder {
		class Neuron;
		class NavMesh;
	}  // namespace AIPathFinder


	namespace Runtime {

		struct Vec2;

		struct Vec2Int;

		struct Render_info;

		template <typename T>
		class xRect;

		typedef xRect<int> Rect;
		typedef xRect<float> Rectf_t;

		class GC;

		// base
		class Object;
		class Component;

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

		class Physics2D;

		struct Color;

		class Mathf;

	}  // namespace Runtime
}  // namespace RoninEngine

#include "Types.h"
#include "LevelEntity.h"
