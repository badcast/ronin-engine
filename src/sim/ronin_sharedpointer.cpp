#include "ronin.h"

using namespace RoninEngine::Runtime;

namespace RoninEngine::Runtime
{
    void RoninPointer::instance_end(RoninPointer* pointer)
    {
        RefReleaseHard(pointer);
    }
           ////////////
           /// Destroys
           ////////////
    void RefReleaseSoft(RoninPointer* object)
    {
        // No longer needed
    }

    void RefReleaseHard(RoninPointer *object)
    {
        if(object->_handle != RefClassType::Const)
            RoninMemory::free(object);
    }

    RoninPointer::RoninPointer() : _handle(RefClassType::Shared), _ref_count_intrusive(0)
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

    // RefRegister removed

    template<>
    Ref<RoninPointer> RoninPointer::GetRef()
    {
        return GetRefMain(this);
    }

    Ref<RoninPointer> RoninPointer::GetRefMain(RoninPointer* pointer)
    {
        if(pointer)
        {
            return Ref<RoninPointer>(pointer);
        }
        return nullptr;
    }

}

