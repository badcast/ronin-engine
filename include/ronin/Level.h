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

    private:
        int globalID;
        bool m_isUnload;
        int _destroyed;
        std::string m_name;
        std::list<Behaviour*>* _firstRunScripts;
        std::list<Behaviour*>* _realtimeScripts;

        std::map<float, std::set<Object*>>* _destructTasks;
        std::unordered_map<Vec2Int, std::set<Transform*>> matrixWorld;

        std::list<Renderer*> _assoc_renderers;
        std::list<Light*> _assoc_lightings;

        std::map<Object*, float> _objects;
        std::list<GameObject*> _gameObjects;

        void intenal_bind_script(Behaviour* obj);
        void push_render_object(Renderer* rend);
        void push_light_object(Light* light);
        void push_object(Object* obj);

        std::vector<Transform*>* get_hierarchy(Transform* parent);

    protected:
        UI::GUI* ui;
        virtual void destructs();

        virtual void level_render_world(SDL_Renderer* renderer);
        virtual void level_render_ui(SDL_Renderer* renderer);
        virtual void level_render_world_late(SDL_Renderer* renderer);
        void matrix_nature(Transform* target, Vec2Int lastPoint);
        void matrix_nature(Transform* target, const Vec2Int& newPoint, const Vec2Int& lastPoint);
        void matrix_nature_pickup(Transform *target);

        virtual void awake();
        virtual void start();
        virtual void update();
        virtual void lateUpdate();
        virtual void onDrawGizmos();
        virtual void onUnloading();

    public:
        // Main or Root object
        GameObject* main_object;

        Level();
        Level(const std::string& name);

        virtual ~Level();

        std::string& name();

        bool is_hierarchy();

        UI::GUI* gui();
        void unload();

        int get_destroyed_frames();

        bool has_destruction_state(Object* obj);

        static std::list<Transform*> matrixCheckDamaged();
        static int matrix_restore();
        static int matrix_restore(const std::list<Transform*>& damaged_content);

        static void get_render_info(int* culled, int* fullobjects);

        static Level* self();
    };

} // namespace RoninEngine
