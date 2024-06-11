#include "ronin.h"

using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    template <>
    const TRect<float> TRect<float>::zero = {};
    template <>
    const TRect<float> TRect<float>::one = {1, 1, 1, 1};

    template <>
    const TRect<int> TRect<int>::zero = {};
    template <>
    const TRect<int> TRect<int>::one = {1, 1, 1, 1};

} // namespace RoninEngine::Runtime

void check_object(RoninEngine::Runtime::Object *obj)
{
    if(obj->_type_ == nullptr)
    {
        throw ronin_null_error();
    }
}
