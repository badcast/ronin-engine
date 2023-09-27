#include "ronin.h"

using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    template <>
    const RRect<float> RRect<float>::zero = {};
    template <>
    const RRect<float> RRect<float>::one = {1, 1, 1, 1};

    template <>
    const RRect<int> RRect<int>::zero = {};
    template <>
    const RRect<int> RRect<int>::one = {1, 1, 1, 1};

} // namespace RoninEngine::Runtime

void check_object(RoninEngine::Runtime::Object *obj)
{
    if(obj->_type_ == nullptr)
    {
        throw ronin_null_error();
    }
}
