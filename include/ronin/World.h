#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{

    class RONIN_API World
    {
    private:
        struct WorldResources *internal_resources = nullptr;

        void internal_bind_script(Behaviour *);
        void push_light_object(Light *);
        void push_object(Object *);

    protected:
        std::string m_name;
        virtual void runtime_destructs();

        virtual void level_render_world();
        virtual void level_render_world_late();

        virtual void OnAwake();
        virtual void OnStart();
        virtual void OnUpdate();
        virtual void OnGizmos();
        virtual void OnUnloading();

    public:
        World() = delete;
        World(const std::string &name);

        virtual ~World();

        std::string &name();

        UI::GUI *getGUI();

        bool isHierarchy();

        void RequestUnload();

        int GetCulled();
        int GetDestroyedFrames();

        std::list<GameObject *> GetAllGameObjects();
        std::list<Component *> GetAllComponents();

        template <typename T>
        std::list<T *> FindObjectsWithType();

        const bool CancelObjectDestruction(GameObject *obj);
        const int CostObjectDestruction(GameObject *obj);
        const bool StateObjectDestruction(GameObject *obj);
        const int CountObjectDestruction();

        std::list<Transform *> matrix_check_damaged();
        int matrix_restore();
        int matrix_restore(const std::list<Transform *> &damaged_content);
        int matrix_count_cache();
        int matrix_clear_cache();

        std::string get_hierarchy_as_tree() const;

        static World *self();
    };

    template <typename T>
    std::list<T *> World::FindObjectsWithType()
    {
        T *_target;
        std::list<T *> __classes;

        std::list<Component *> __compr = GetAllComponents();

        for(auto iter = __compr.begin(); iter != __compr.end(); ++iter)
        {
            if((_target = dynamic_cast<T *>(*iter)))
            {
                __classes.emplace_back(_target);
            }
        }

        return __classes;
    }

} // namespace RoninEngine::Runtime
