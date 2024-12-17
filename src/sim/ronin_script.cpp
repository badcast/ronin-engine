#include "ronin.h"

namespace RoninEngine::Runtime
{
    Behaviour::Behaviour() : Behaviour(DESCRIBE_AS_MAIN_OFF(Behaviour))
    {
    }

    Behaviour::Behaviour(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(Behaviour))
    {
        DESCRIBE_AS_MAIN(Behaviour);
    }

    void GameObject::bind_script(BindType bindFlags, BehaviourRef script)
    {
        if(!script || !bindFlags)
            return;
        if((bindFlags & Bind_Start))
            currentWorld->irs->runtimeScriptBinders[Bind_Start].insert(script.ptr_);
        if((bindFlags & Bind_Update))
            currentWorld->irs->runtimeScriptBinders[Bind_Update].insert(script.ptr_);
        if((bindFlags & Bind_LateUpdate))
            currentWorld->irs->runtimeScriptBinders[Bind_LateUpdate].insert(script.ptr_);
        if((bindFlags & Bind_Gizmos))
            currentWorld->irs->runtimeScriptBinders[Bind_Gizmos].insert(script.ptr_);
    }

           ///////////////////////////////////////////
           ///////////////////////////////////////////
           ///////////////////////////////////////////
           ///////////////////////////////////////////
           ///////////////////////////////////////////
    void Behaviour::OnAwake() ////////////
    {                         ////////////
    } ////////////
    void Behaviour::OnStart() ////////////
    {                         ////////////
    } ////////////
    void Behaviour::OnUpdate() ////////////
    {                          ////////////
    } ////////////
    void Behaviour::OnLateUpdate() ////////////
    {                              ////////////
    } ////////////
    void Behaviour::OnGizmos() ////////////
    {                          ////////////
    } ////////////
    void Behaviour::OnDestroy() ////////////
    {                           ////////////
    } ////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    void scripts_start()
    {
        std::set<Behaviour*> __last;
        __last.merge(currentWorld->irs->runtimeScriptBinders[GameObject::BindType::Bind_Start]);
        currentWorld->irs->runtimeScriptBinders[GameObject::BindType::Bind_Start].clear();
        for(Behaviour * script : __last)
            script->OnStart();
    }

    void scripts_update()
    {
        for(Behaviour *script : currentWorld->irs->runtimeScriptBinders[GameObject::BindType::Bind_Update])
                script->OnUpdate();
    }

    void scripts_lateUpdate()
    {
        for(Behaviour * script : currentWorld->irs->runtimeScriptBinders[GameObject::BindType::Bind_LateUpdate])
            script->OnLateUpdate();
    }

    void scripts_gizmos()
    {
        for(Behaviour * script : currentWorld->irs->runtimeScriptBinders[GameObject::BindType::Bind_Gizmos])
            script->OnGizmos();
    }

    void scripts_unbind(BehaviourRef script)
    {
        if(!script)
            return;
        for(std::map<GameObject::BindType, std::set<Behaviour*>>::iterator iter = std::begin(currentWorld->irs->runtimeScriptBinders); iter != std::end(currentWorld->irs->runtimeScriptBinders); ++iter)
        {
            iter->second.erase(script.ptr_);
        }
    }

} // namespace RoninEngine::Runtime
