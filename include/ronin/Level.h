#pragma once
#include "begin.h"

namespace RoninEngine
{
    class RONIN_API Level
    {
        friend class Application;
        friend class UI::GUI;
        friend class Runtime::Object;
        friend class Runtime::Transform;
        friend class Runtime::GameObject;
        friend class Runtime::Renderer;
        friend class Runtime::Camera;
        friend class Runtime::Camera2D;
        friend class Runtime::Physics2D;

        friend bool Runtime::instanced(Runtime::Object* obj);
        friend void Runtime::destroy(Runtime::Object* obj);
        friend void Runtime::destroy(Runtime::Object* obj, float t);
        friend void Runtime::destroy_immediate(Runtime::Object* obj);

    private:
        int globalID;
        bool m_isUnload;
        int _destroyed;
        std::string m_name;
        std::list<Runtime::Behaviour*>* _firstRunScripts;
        std::list<Runtime::Behaviour*>* _realtimeScripts;

        std::map<float, std::set<Runtime::Object*>>* _destructTasks;
        std::unordered_map<Runtime::Vec2Int, std::set<Runtime::Transform*>> matrixWorld;

        std::list<Runtime::Renderer*> _assoc_renderers;
        std::list<Runtime::Light*> _assoc_lightings;

        std::map<Runtime::Object*, float> _objects;
        std::list<Runtime::GameObject*> _gameObjects;

        void intenal_bind_script(Runtime::Behaviour* obj);
        void push_render_object(Runtime::Renderer* rend);
        void push_light_object(Runtime::Light* light);
        void push_object(Runtime::Object* obj);

        std::vector<Runtime::Transform*>* get_hierarchy(Runtime::Transform* parent);

    protected:
        UI::GUI* ui;
        virtual void destructs();

        virtual void level_render_world(SDL_Renderer* renderer);
        virtual void level_render_ui(SDL_Renderer* renderer);
        virtual void level_render_world_late(SDL_Renderer* renderer);
        void matrix_nature(Runtime::Transform* target, Runtime::Vec2Int lastPoint);
        void matrix_nature(Runtime::Transform* target, const Runtime::Vec2Int& newPoint, const Runtime::Vec2Int& lastPoint);
        void matrix_nature_pickup(Runtime::Transform *target);

        virtual void awake();
        virtual void start();
        virtual void update();
        virtual void lateUpdate();
        virtual void onDrawGizmos();
        virtual void onUnloading();

    public:
        // Main or Root object
        Runtime::GameObject* main_object;

        Level();
        Level(const std::string& name);

        virtual ~Level();

        std::string& name();

        bool is_hierarchy();

        UI::GUI* gui();
        void unload();

        int get_destroyed_frames();

        static std::list<Runtime::Transform*> matrixCheckDamaged();
        static int matrix_restore();
        static int matrix_restore(const std::list<Runtime::Transform*>& damaged_content);

        static void get_render_info(int* culled, int* fullobjects);

        static Level* self();
    };

} // namespace RoninEngine
