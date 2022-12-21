 // Pre declarations
namespace RoninEngine {
class Application;
class Level;

namespace UI {
struct UIElement;
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

}  // namespace Runtime
}  // namespace RoninEngine
