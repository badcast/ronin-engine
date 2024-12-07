#include "ronin.h"

using namespace RoninEngine::Runtime;

namespace RoninEngine::Runtime
{
    ////////////////
    // Destroys
    ////////////////
    RoninPointer* RefNoFree(RoninPointer * object)
    {
        object->_handle = RefClassType::Const;
        return object;
    }

    void release_pointer(RoninPointer* object)
    {
        if(object == nullptr)
            return;
        if(object->_handle == RefClassType::Const)
            object->_handle = RefClassType::Null;
        else
            RoninMemory::free(object);
    }

    void unref(ComponentRef& object){
    }
    void unref(GameObjectRef& object){
    }
    void unref(AtlasRef& object){
    }
    void unref(SpriteRef& object){
    }

    RoninPointer::RoninPointer()
    {
        _handle = RefClassType::Shared;
    }

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

    Ref<RoninPointer> GetRefMain(RoninPointer* pointer)
    {
        std::unordered_map<RoninPointer*, Ref<RoninPointer>>::iterator iter = currentWorld->irs->refPointers.find(pointer);
        if(iter != currentWorld->irs->refPointers.end())
            return iter->second;
        return Ref<RoninPointer>{nullptr};
    }

    template<>
    Ref<RoninPointer> RoninPointer::GetRef()
    {
        return GetRefMain(this);
    }
}

