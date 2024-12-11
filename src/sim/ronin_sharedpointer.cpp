#include "ronin.h"

using namespace RoninEngine::Runtime;

namespace RoninEngine::Runtime
{
    void instance_end(RoninPointer* pointer)
    {
        RefReleaseHard(pointer);
    }

    RoninPointer* RefNoFree(RoninPointer * object)
    {
        object->_handle = RefClassType::Const;
        return object;
    }

           ////////////
           /// Destroys
           ////////////
    void RefReleaseSoft(RoninPointer* object)
    {
        std::unordered_map<RoninPointer*, Ref<RoninPointer>>::iterator iter;
        iter = currentWorld->irs->refPointers.find(object);
        if(iter != currentWorld->irs->refPointers.end())
            currentWorld->irs->refPointers.erase(iter);
    }

    void RefReleaseHard(RoninPointer *object)
    {
        if(object->_handle != RefClassType::Const)
            RoninMemory::free(object);
    }

    RoninPointer::RoninPointer() : _handle(RefClassType::Shared)
    {}

    RoninPointer::~RoninPointer()
    {}

    bool RoninPointer::isNull() const
    {
        return _handle == RefClassType::Null;
    }

    RoninPointer::operator bool() const
    {
        return _handle != RefClassType::Null;
    }

    void RefRegister(Ref<RoninPointer> noninitref)
    {
        if(currentWorld == nullptr || currentWorld->irs == nullptr || !noninitref)
            return;
        currentWorld->irs->refPointers[static_cast<RoninPointer*>(noninitref.ptr_)] = noninitref;
    }

    Ref<RoninPointer> GetRefMain(RoninPointer* pointer)
    {
        if(pointer)
        {
            std::unordered_map<RoninPointer*, Ref<RoninPointer>>::iterator iter = currentWorld->irs->refPointers.find(pointer);
            if(iter != currentWorld->irs->refPointers.end())
                return iter->second;
        }
        return nullptr;
    }

    template<>
    Ref<RoninPointer> RoninPointer::GetRef()
    {
        return GetRefMain(this);
    }
}

