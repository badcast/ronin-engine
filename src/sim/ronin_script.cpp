#include "ronin.h"

#define BIND(FLAG, SCRIPT) \
    if(bindFlags & FLAG)   \
        binder[FLAG].insert(SCRIPT);

#define RUN_SCRIPTS(FLAG, METHOD)                                             \
    for(Behaviour * script : switched_world->irs->runtimeScriptBinders[FLAG]) \
        script->METHOD();

namespace RoninEngine::Runtime
{
    Behaviour::Behaviour() : Behaviour(DESCRIBE_AS_MAIN_OFF(Behaviour))
    {
    }

    Behaviour::Behaviour(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(Behaviour))
    {
        DESCRIBE_AS_MAIN(Behaviour);
    }

    void GameObject::bind_script(BindType bindFlags, Behaviour *script)
    {
        auto &binder = switched_world->irs->runtimeScriptBinders;

        BIND(Bind_Start, script);
        BIND(Bind_Update, script);
        BIND(Bind_LateUpdate, script);
        BIND(Bind_Gizmos, script);
    }

    ///////////////////////////////////////////
    void Behaviour::OnAwake()      ////////////
    {                              ////////////
    }                              ////////////
    void Behaviour::OnStart()      ////////////
    {                              ////////////
    }                              ////////////
    void Behaviour::OnUpdate()     ////////////
    {                              ////////////
    }                              ////////////
    void Behaviour::OnLateUpdate() ////////////
    {                              ////////////
    }                              ////////////
    void Behaviour::OnGizmos()     ////////////
    {                              ////////////
    }                              ////////////
    void Behaviour::OnDestroy()    ////////////
    {                              ////////////
    }                              ////////////
    ///////////////////////////////////////////

    void scripts_start()
    {
        std::set<Behaviour *> __last;
        __last.merge(switched_world->irs->runtimeScriptBinders[GameObject::BindType::Bind_Start]);

        for(Behaviour *script : __last)
            script->OnStart();
    }

    void scripts_update()
    {
        RUN_SCRIPTS(GameObject::BindType::Bind_Update, OnUpdate);
    }

    void scripts_lateUpdate()
    {
        RUN_SCRIPTS(GameObject::BindType::Bind_LateUpdate, OnLateUpdate);
    }

    void scripts_gizmos()
    {
        RUN_SCRIPTS(GameObject::BindType::Bind_Gizmos, OnGizmos);
    }

    void scripts_unbind(Behaviour *script)
    {
        for(auto iter = std::begin(switched_world->irs->runtimeScriptBinders); iter != std::end(switched_world->irs->runtimeScriptBinders);
            ++iter)
        {
            iter->second.erase(script);
        }
    }

} // namespace RoninEngine::Runtime

#undef BIND
#undef RUN_SCRIPTS
