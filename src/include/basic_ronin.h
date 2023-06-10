/*
   This is file describe type
*/

#pragma once

#include "ronin.h"

namespace std
{
    // this for Hashtable function

    template <>
    struct hash<RoninEngine::Runtime::Vec2> {
        std::size_t operator()(const RoninEngine::Runtime::Vec2& val) const noexcept
        {
            std::int64_t fake = (*reinterpret_cast<std::int64_t*>(&const_cast<RoninEngine::Runtime::Vec2&>(val)));
            return std::hash<std::int64_t> {}(fake);
        }
    };

    template <>
    struct hash<RoninEngine::Runtime::Vec2Int> {
        std::size_t operator()(const RoninEngine::Runtime::Vec2Int& val) const noexcept
        {
            std::int64_t fake = (*reinterpret_cast<std::int64_t*>(&const_cast<RoninEngine::Runtime::Vec2Int&>(val)));
            return std::hash<std::int64_t> {}(fake);
        }
    };

} // namespace std

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
static std::enable_if_t<std::is_base_of<RoninEngine::Runtime::Object, T>::value, const char*> runtime_define_type(const char* typestr, T* self, char** _type_, const char* name)
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

namespace RoninEngine
{
    namespace UI
    {
        struct UIElement {
            Runtime::Rect rect;
            Runtime::Rect contextRect;
            std::uint8_t options;
            uid id;
            uid parentId;
            std::string text;
            GUIControlPresents prototype;
            std::list<uint8_t> childs;
            void* resources;
            void* event;
        };
        struct GUIResources {
            struct {
                // controls
                std::vector<UIElement> elements;
                std::list<uid> layers;
                uid focusedID;
            } ui_layer;

            ui_callback callback;
            void* callbackData;
            World* __level_owner;
            bool hitCast;
            bool _focusedUI;
            bool visible;
        };

    }
    namespace Runtime
    {
        struct Rendering {
            Rect src;
            Rectf dst;
            SDL_Texture* texture;
            SDL_Renderer* renderer;
        };

        struct WorldResources {
            // unique ids for Object types in current world
            std::uint32_t _level_ids_;

            // state is unloading
            bool request_unloading;

            std::list<Sprite*> offload_sprites;
            std::list<SDL_Surface*> offload_surfaces;
            std::list<AudioClip> offload_audioclips;
            // destroyed queue object
            int _destroyed;

            // destroy last delaying for action
            float _destroy_delay_time;

            std::list<Behaviour*>* _firstRunScripts;
            std::list<Behaviour*>* _realtimeScripts;

            // destruction task (queue object)
            std::map<float, std::set<Object*>>* _destructTasks;
            std::unordered_map<Vec2Int, std::set<Transform*>> matrixWorld;

            std::list<Light*> _assoc_lightings;

            // World object (use)
            std::set<Object*> world_objects;

            // Main UI Object
            UI::GUI* gui = nullptr;

            // Main or Root object
            GameObject* main_object = nullptr;
        };

        struct AudioClip {
            Mix_Chunk* mix_chunk;
            Mix_Music* mix_music;
        };

        struct AudioSourceData {
            AudioClip* m_clip;
            float m_volume;
        };

        // pre-decloration
        void load_world(World*);
        bool unload_world(World*);
        Transform* get_root(World*);

    }
}
