#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{

    class RONIN_API World
    {
    private:
        struct WorldResources *irs;
        std::string m_name;

    protected:
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

        UI::GUI *GetGUI();

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

        std::list<Transform *> MatrixCheckDamage();
        int MatrixRestore();
        int MatrixRestore(const std::list<Transform *> &damaged_content);
        int MatrixCacheCount();
        int MatrixCacheClear();

        // TODO: Make function for moving out GameObject into Other World
        void ObjectMoveTo(GameObject *target, World *newWorld);

        std::string GetTreeOfHierarchy() const;

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
