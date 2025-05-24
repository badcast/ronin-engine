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

        std::list<GameObjectRef> GetAllGameObjects();
        std::list<ComponentRef> GetAllComponents();

        template <typename T>
        std::list<Ref<T>> FindObjectsWithType();

        const bool CancelObjectDestruction(GameObjectRef obj);
        const int CostObjectDestruction(GameObjectRef obj);
        const bool StateObjectDestruction(GameObjectRef obj);
        const int CountObjectDestruction();

        std::list<Transform *> MatrixCheckDamage();
        int MatrixRestore();
        int MatrixRestore(const std::list<Transform *> &damaged_content);
        int MatrixCacheCount();
        int MatrixCacheClear();
        std::uint64_t StatCountRefLinks();
        std::uint64_t StatMaxRefDepth();

        // TODO: Make function for moving out GameObject into Other World
        void ObjectMoveTo(GameObject *target, World *newWorld);

        std::string GetTreeOfHierarchy() const;

        static World *GetCurrentWorld();
    };

    template <typename T>
    std::list<Ref<T>> World::FindObjectsWithType()
    {
        Ref<T> _target;
        std::list<Ref<T>> __classes;
        std::list<ComponentRef> __compr = GetAllComponents();
        for(auto iter = __compr.begin(); iter != __compr.end(); ++iter)
        {
            if((_target = DynamicCast<T>(*iter)))
            {
                __classes.emplace_back(_target);
            }
        }

        return __classes;
    }

} // namespace RoninEngine::Runtime
