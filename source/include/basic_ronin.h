/*
   This is file describe type
*/

#pragma once

#include "ronin.h"

static std::unordered_map<std::type_index, const char*> main_ronin_types;

template <typename base, typename _derived>
constexpr bool object_base_of()
{
    return std::is_base_of<base, _derived>();
}

template <typename base, typename _derived>
constexpr bool object_base_of(base* obj, _derived* compare)
{
    return std::is_base_of<base, _derived>();
}

// this method for replacing C++ dynamic_cast
template <typename T>
static typename std::enable_if<std::is_base_of<RoninEngine::Runtime::Object, T>::value, const char*>::type runtime_define_type(const char* typestr, T* self, char** _type_, const char* name)
{
    // param @self not use
    if (_type_ != nullptr && *_type_ == nullptr) {
        std::type_index index = std::type_index(typeid(T));
        if (main_ronin_types.find(index) == std::end(main_ronin_types))
            main_ronin_types[index] = typestr;

        // set unique type (for delete dynamic_cast)
        *_type_ = (char*)typestr;
    }
    if (name == nullptr)
        return typestr;
    else
        return name;
}

extern void check_object(RoninEngine::Runtime::Object* obj);

#define DESCRIBE_TYPE(TYPE, self, _type_, name) (runtime_define_type<TYPE>(#TYPE, self, (_type_), name))

#define DESCRIBE_AS_ONLY_NAME(TYPE) (DESCRIBE_TYPE(TYPE, this, &_type_, name.c_str()))

#define DESCRIBE_AS_MAIN_OFF(TYPE) (DESCRIBE_TYPE(TYPE, this, &_type_, nullptr))

#define DESCRIBE_AS_MAIN(TYPE) (DESCRIBE_TYPE(TYPE, this, &_type_, name.c_str()))

namespace RoninEngine::Runtime
{
    struct Rendering {
        Rect src;
        Rectf dst;
        SDL_Texture* texture;
        SDL_Renderer* renderer;
    };
}
