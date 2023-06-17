#include "ronin.h"
using namespace RoninEngine::Exception;
namespace RoninEngine::Runtime
{
    template <>
    const RRect<float> RRect<float>::zero = {};
    template <>
    const RRect<float> RRect<float>::one = { 1, 1, 1, 1 };

    template <>
    const RRect<int> RRect<int>::zero = {};
    template <>
    const RRect<int> RRect<int>::one = { 1, 1, 1, 1 };

} // namespace RoninEngine

// template<> RONIN_API const char * define_type<RoninEngine::Runtime::GameObject>(char const*,              RoninEngine::Runtime::GameObject*, int*);
// template<> RONIN_API const char * define_type<RoninEngine::Runtime::Player>(char const*,              RoninEngine::Runtime::Player*, int*);
// template<> RONIN_API const char * define_type<RoninEngine::Runtime::Camera2D>(char const*,              RoninEngine::Runtime::Camera2D*, int*);
// template<> RONIN_API const char * define_type<RoninEngine::Runtime::Spotlight>(char const*,              RoninEngine::Runtime::Spotlight*, int*);
// template<> RONIN_API const char * define_type<RoninEngine::Runtime::Transform>(char const*,              RoninEngine::Runtime::Transform*, int*);
// template<> RONIN_API const char * define_type<RoninEngine::Runtime::Object>(char const*,              RoninEngine::Runtime::Object*, int*);
// template<> RONIN_API const char * define_type<RoninEngine::Runtime::Terrain2D>(char const*,              RoninEngine::Runtime::Terrain2D*, int*);
// template<> RONIN_API const char * define_type<RoninEngine::Runtime::Renderer>(char const*,              RoninEngine::Runtime::Renderer*, int*);
// template<> RONIN_API const char * define_type<RoninEngine::Runtime::Component>(char const*,              RoninEngine::Runtime::Component*, int*);
// template<> RONIN_API const char * define_type<RoninEngine::Runtime::SpriteRenderer>(char const*,              RoninEngine::Runtime::SpriteRenderer*, int*);
// template<> RONIN_API const char * define_type<RoninEngine::Runtime::Camera>(char const*,              RoninEngine::Runtime::Camera*, int*);
// template<> RONIN_API const char * define_type<RoninEngine::Runtime::Light>(char const*,              RoninEngine::Runtime::Light*, int*);

void check_object(RoninEngine::Runtime::Object* obj)
{
    if (obj->_type_ == nullptr) {
        throw ronin_null_error();
    }
}
