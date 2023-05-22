#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Level
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

        friend bool instanced(Object* obj);
        friend void destroy(Object* obj);
        friend void destroy(Object* obj, float t);
        friend void destroy_immediate(Object* obj);

        friend bool unload_level(Level* level);

    private:
        std::uint32_t _level_ids_;
        bool request_unloading;
        int _destroyed;
        float _destroy_delay_time;
        std::string m_name;
        std::list<Behaviour*>* _firstRunScripts;
        std::list<Behaviour*>* _realtimeScripts;

        std::map<float, std::set<Object*>>* _destructTasks;
        std::unordered_map<Vec2Int, std::set<Transform*>> matrixWorld;

        std::list<Light*> _assoc_lightings;

        std::map<Object*, float> _objects;

        void intenal_bind_script(Behaviour* obj);
        void push_light_object(Light* light);
        void push_object(Object* obj);

        static std::list<Transform*>* get_hierarchy(Transform* parent);

    protected:
        UI::GUI* ui;
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
        // Main or Root object
        GameObject* main_object;

        Level();
        Level(const std::string& name);

        virtual ~Level();

        std::string& name();

        UI::GUI* get_gui();

        bool is_hierarchy();

        void request_unload();

        int get_destroyed_frames();

        const bool object_desctruction_cancel(Object* obj);
        const int object_destruction_cost(Object* obj);
        const bool object_destruction_state(Object* obj);
        const int object_destruction_count();

        static std::list<Transform*> matrix_check_damaged();
        static int matrix_restore();
        static int matrix_restore(const std::list<Transform*>& damaged_content);

        static void get_render_info(int* culled, int* fullobjects);

        static Level* self();
    };

} // namespace RoninEngine
