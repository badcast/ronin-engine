#include "ronin.h"

using namespace RoninEngine::Runtime;

namespace RoninEngine::Runtime
{
#if 0
#define INSTANCEA( T, U )\
        template Ref<U> AReinterpretCast(Ref<T>);\
        template Ref<U> AStaticCast(Ref<T>);\
        template Ref<U> ADynamicCast(Ref<T>)

#define INSTANCE(U,T)\
    INSTANCEA(T,U); \
    INSTANCEA(U,T);

    INSTANCE(Object, Component);
    INSTANCE(Renderer, Component);
    INSTANCE(Transform, Component);
    INSTANCE(Behaviour, Component);
    INSTANCE(Collision, Component);
    INSTANCE(Light, Component);
    INSTANCE(Spotlight, Component);
    INSTANCE(SpriteRenderer, Component);
    INSTANCE(Terrain2D, Component);
    INSTANCE(Camera, Component);
    INSTANCE(Camera2D, Component);
    INSTANCE(MoveController2D, Component);
    INSTANCE(AudioSource, Component);
    INSTANCE(ParticleSystem, Component);
    INSTANCE(GameObject, RoninPointer);
    INSTANCE(Sprite, RoninPointer);
    INSTANCE(Atlas, RoninPointer);

#undef INSTANCE
#undef INSTANCEA
#endif
}

////////////////
// Destroys
////////////////

void ReleasePointer(RoninPointer* object)
{
    if(object == nullptr || object->isNull())
        return;

    object->_handle = nullptr;
    RoninMemory::_cut_oop_from(object);
}

void ReleaseRef(ComponentRef& object){


    // ReleasePointer(static_cast<RoninPointer*>(object.ptr_));
}
void ReleaseRef(GameObjectRef& object){


      //ReleasePointer(static_cast<RoninPointer*>(object.ptr_));
}
void ReleaseRef(AtlasRef& object){


      //ReleasePointer(static_cast<RoninPointer*>(object.ptr_));
}
void ReleaseRef(SpriteRef& object){


      //ReleasePointer(static_cast<RoninPointer*>(object.ptr_));
}

RoninPointer::RoninPointer()
{
}

RoninPointer::~RoninPointer()
{
    ReleasePointer(this);
}

bool RoninPointer::isNull() const
{
    return _handle == nullptr;
}

RoninPointer::operator bool() const
{
    return !isNull();
}

template<>
Ref<RoninPointer> RoninPointer::GetRef()
{
    std::unordered_map<RoninPointer*, Ref<RoninPointer>>::iterator iter = currentWorld->irs->refPointers.find(this);
    if(iter != currentWorld->irs->refPointers.end())
        return iter->second;
    return Ref<RoninPointer>{nullptr};
}

