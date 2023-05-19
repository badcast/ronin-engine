/*
   This is file describe type
*/

#pragma once

#include "ronin.h"

enum { UNDEFINED_TYPE = -1 };

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

template <typename T>
static const char* define_type(const char* typestr, T* self, long* _type_)
{
    // param @self not use
    if (_type_ && *_type_ == UNDEFINED_TYPE) {
        std::type_index index = std::type_index(typeid(T));
        if (main_ronin_types.find(index) == std::end(main_ronin_types)) {
            main_ronin_types[index] = typestr;
        }
        // set unique type (for delete dynamic_cast)

        memcpy(_type_, (const void*)&typestr, sizeof(long));
    }
    return typestr;
}

extern void check_object(RoninEngine::Runtime::Object* obj);

#define DESCRIBE_TYPE(type, self, _type_) (define_type<type>(#type, self, (_type_)))

namespace RoninEngine::Runtime
{
    struct Rendering {
        Rect src;
        Rectf_t dst;
        SDL_Texture* texture;
        SDL_Renderer* renderer;
    };
}
