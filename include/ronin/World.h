#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API World
    {
        friend class RoninEngine::Application;
        friend class RoninEngine::UI::GUI;
        friend class Object;
        friend class Transform;
        friend class GameObject;
        friend class Renderer;
        friend class Camera;
        friend class Camera2D;
        friend class Physics2D;
        friend class Primitive;

        friend bool instanced(Object* obj);
        friend void destroy(Object* obj);
        friend void destroy(Object* obj, float t);
        friend void destroy_immediate(Object* obj);

        friend void load_world(World*);
        friend bool unload_world(World*);
        friend Transform* get_root(World*);

    private:
        void intenal_bind_script(Behaviour* obj);
        void push_light_object(Light* light);
        void push_object(Object* obj);

        static std::list<Transform*>* get_hierarchy(Transform* parent);

    protected:
        struct WorldResources* internal_resources = nullptr;

        std::string m_name;
        virtual void runtime_destructs();

        virtual void level_render_world(SDL_Renderer* renderer, RoninEngine::ScoreWatcher* watcher);
        virtual void level_render_ui(SDL_Renderer* renderer);
        virtual void level_render_world_late(SDL_Renderer* renderer);
        void matrix_nature(Transform* target, Vec2Int lastPoint);
        void matrix_nature(Transform* target, const Vec2Int& newPoint, const Vec2Int& lastPoint);
        void matrix_nature_pickup(Transform* target);

        virtual void on_awake();
        virtual void on_start();
        virtual void on_update();
        virtual void on_late_update();
        virtual void on_gizmo();
        virtual void on_unloading();

    public:
        World() = delete;
        World(const std::string& name);

        virtual ~World();

        std::string& name();

        UI::GUI* get_gui();

        bool is_hierarchy();

        void request_unload();

        int get_destroyed_frames();

        std::list<GameObject*> get_all_gameobjects();
        std::list<Component*> get_all_components();

        template <typename T>
        std::list<T*> find_objects_with_type();

        const bool object_desctruction_cancel(Object* obj);
        const int object_destruction_cost(Object* obj);
        const bool object_destruction_state(Object* obj);
        const int object_destruction_count();

        static std::list<Transform*> matrix_check_damaged();
        static int matrix_restore();
        static int matrix_restore(const std::list<Transform*>& damaged_content);

        static void get_render_info(int* culled, int* fullobjects);

        static World* self();
    };

    template <typename T>
    std::list<T*> World::find_objects_with_type()
    {
        T* _target;
        std::list<T*> __classes;

        std::list<Component*> __compr = get_all_components();

        for (auto iter = __compr.begin(); iter != __compr.end(); ++iter) {
            if ((_target = dynamic_cast<T*>(*iter))) {
                __classes.emplace_back(_target);
            }
        }

        return __classes;
    }

} // namespace RoninEngine
