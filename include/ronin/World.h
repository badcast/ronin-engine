#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{

    class RONIN_API World
    {
    private:
        struct WorldResources* internal_resources = nullptr;

        void internal_bind_script(Behaviour*);
        void push_light_object(Light*);
        void push_object(Object*);

    protected:
        std::string m_name;
        virtual void runtime_destructs();

        virtual void level_render_world(SDL_Renderer* renderer, RoninEngine::ScoreWatcher* watcher);
        virtual void level_render_world_late(SDL_Renderer* renderer);
        void matrix_nature(Transform* target, Vec2Int lastPoint);
        void matrix_nature(Transform* target, const Vec2Int& newPoint, const Vec2Int& lastPoint);
        void matrix_nature_pickup(Transform* target);

        virtual void on_awake();
        virtual void on_start();
        virtual void on_update();
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

        int get_culled();
        int get_destroyed_frames();

        std::list<GameObject*> get_all_gameobjects();
        std::list<Component*> get_all_components();

        template <typename T>
        std::list<T*> find_objects_with_type();

        const bool object_desctruction_cancel(GameObject* obj);
        const int object_destruction_cost(GameObject* obj);
        const bool object_destruction_state(GameObject* obj);
        const int object_destruction_count();

        std::list<Transform*> matrix_check_damaged();
        int matrix_restore();
        int matrix_restore(const std::list<Transform*>& damaged_content);
        int matrix_clear_cache();

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
