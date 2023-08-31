/*
   This is file describe type
*/

#pragma once

#include "ronin.h"

static std::unordered_map<std::type_index, const char *> main_ronin_types;

// this method for replacing C++ dynamic_cast
template <typename T>
static std::enable_if_t<std::is_base_of<RoninEngine::Runtime::Object, T>::value, const char *> runtime_define_type(
    const char *typestr, T *self, char **_type_, const char *name)
{
    // param @self not use
    if(_type_ != nullptr && *_type_ == nullptr)
    {
        std::type_index index {typeid(T)};
        if(main_ronin_types.find(index) == std::end(main_ronin_types))
            main_ronin_types[index] = typestr;

        // set unique type (for delete dynamic_cast)
        *_type_ = (char *) typestr;
    }
    if(name == nullptr)
        return typestr;
    else
        return name;
}

extern void check_object(RoninEngine::Runtime::Object *obj);

#ifndef NDEBUG
#define TEST_MALLOC 0
#endif

#define DESCRIBE_TYPE(TYPE, self, _type_, name) (runtime_define_type<TYPE>(#TYPE, self, (_type_), name))

#define DESCRIBE_AS_ONLY_NAME(TYPE) (DESCRIBE_TYPE(TYPE, this, &_type_, name.c_str()))

#define DESCRIBE_AS_MAIN_OFF(TYPE) (DESCRIBE_TYPE(TYPE, this, &_type_, nullptr))

#define DESCRIBE_AS_MAIN(TYPE) (DESCRIBE_TYPE(TYPE, this, &_type_, name.c_str()))

namespace std
{
    // this for Hashtable function

    template <>
    struct hash<RoninEngine::Runtime::Vec2>
    {
        std::size_t operator()(const RoninEngine::Runtime::Vec2 &val) const noexcept
        {
            std::int64_t fake = (*reinterpret_cast<std::int64_t *>(&const_cast<RoninEngine::Runtime::Vec2 &>(val)));
            return std::hash<std::int64_t> {}(fake);
        }
    };

    template <>
    struct hash<RoninEngine::Runtime::Vec2Int>
    {
        std::size_t operator()(const RoninEngine::Runtime::Vec2Int &val) const noexcept
        {
            std::int64_t fake = (*reinterpret_cast<std::int64_t *>(&const_cast<RoninEngine::Runtime::Vec2Int &>(val)));
            return std::hash<std::int64_t> {}(fake);
        }
    };

} // namespace std

namespace RoninEngine
{
    extern SDL_Renderer *renderer;
    extern Resolution active_resolution;
    extern bool ronin_debug_mode;
    extern TimingWatcher queue_watcher;

    namespace UI
    {
        enum GUIControlPresents : std::uint8_t
        {
            RGUI_TEXT,
            RGUI_BUTTON,
            RGUI_TEXT_EDIT,
            RGUI_HSLIDER,
            RGUI_VSLIDER,
            RGUI_PICTURE_BOX,
            RGUI_DROPDOWN
        };
        struct ui_resource;
        struct UIElement
        {
            uid id;
            uid parentId;
            std::uint8_t options;
            std::vector<uid> childs;
            Runtime::Rect rect;
            Runtime::Rect contextRect;
            GUIControlPresents prototype;
            std::string text;
            void *resources;
            void *event;
        };
        struct GUIResources
        {
            struct
            {
                // controls
                std::vector<UIElement> elements;
                std::list<uid> layers;
                uid focusedID;
            } ui_layer;

            ui_callback callback;
            void *callbackData;
            Runtime::World *owner;
            bool interactable;
            bool _focusedUI;
            bool visible;
            bool mouse_hover;
        };

        extern void draw_font_at(
            SDL_Renderer *renderer,
            const std::string &text,
            int fontSize,
            Runtime::Vec2Int screenPoint,
            const Runtime::Color color,
            bool alignCenter = false);
    } // namespace UI
    namespace Runtime
    {
        enum class CameraEvent
        {
            CAM_DELETED,
            CAM_TARGET
        };

        enum MouseStateFlags
        {
            MouseDown = 1,
            MouseUp = 2
        };

        struct RoninInput
        {
            int _mouse_wheels;
            std::int8_t _mouse_state[SDL_BUTTON_X2];
            Vec2Int _mouse_position;
            Vec2 _movement_axis;
        };

        struct Rendering
        {
            Rect src;
            Rectf dst;
            SDL_Texture *texture;
        };

        struct CameraResource
        {
            bool targetClear;
            std::map<int, std::vector<Renderer *>> renders;
            std::set<Renderer *> prev;
            std::set<Light *> _lightsOutResults;
        };

        struct AudioClip
        {
            Mix_Chunk *mix_chunk;
#ifndef NDEBUG
            int used;
#endif
            AudioClip(Mix_Chunk *chunk)
                : mix_chunk(chunk)
#ifndef NDEBUG
                  ,
                  used(0)
#endif
            {
            }
        };

        struct MusicClip
        {
            Mix_Music *mix_music;
#ifndef NDEBUG
            int used;
#endif
            MusicClip(Mix_Music *mus)
                : mix_music(mus)
#ifndef NDEBUG
                  ,
                  used(0)
#endif
            {
            }
        };

        struct AudioSourceData
        {
            AudioClip *m_clip;
            int target_channel;
        };

        struct MusicPlayerData
        {
            MusicClip *m_clip;
        };

        struct GidResources
        {
            std::vector<AudioClip *> gid_audio_clips;
            std::vector<MusicClip *> gid_music_clips;
            std::vector<SDL_Surface *> gid_surfaces;
        };

        struct WorldResources
        {
            // unique ids for Object types in current world
            std::uint32_t _level_ids_;

            // state is unloading
            bool request_unloading;

            std::list<Sprite *> offload_sprites;
            std::list<SDL_Surface *> offload_surfaces;

            int audio_channels;
            int audio_reserved_channels = MIX_CHANNELS;

            // destroyed queue object
            int _destroyed;

            // destroy last delaying for action
            float _destroy_delay_time;

            std::list<Behaviour *> *_firstRunScripts;
            std::list<Behaviour *> *_realtimeScripts;

            // destruction task (queue object)
            std::map<float, std::set<GameObject *>> *_destructTasks;

            // Matrix
            std::unordered_map<Vec2Int, std::set<Transform *>> matrix;

            std::list<Light *> _assoc_lightings;

            std::list<CameraResource *> camera_resources;

            // External resources
            GidResources *external_local_resources;

            // World object (use)
            std::set<Object *> world_objects;

            // Main UI Object
            UI::GUI *gui;

            // Main camera for render
            Camera *main_camera;

            // Main or Root object
            GameObject *main_object;

            void event_camera_changed(Camera *target, CameraEvent state);
        };

        // Global variables
        extern World *switched_world;

        void native_render_2D(Camera2D *camera);

        void internal_destroy_object_dyn(Object *obj);

        // TODO: Complete that function for types
        template <typename T, typename std::enable_if<std::is_base_of<Object, T>::value, std::nullptr_t>::type = nullptr>
        void internal_destroy_object(T *obj);

        void load_world(World *);
        bool unload_world(World *);

        void hierarchy_parent_change(Transform *from, Transform *newParent);
        void hierarchy_remove(Transform *from, Transform *off);
        void hierarchy_remove_all(Transform *from);
        void hierarchy_append(Transform *from, Transform *off);
        bool hierarchy_sibiling(Transform *from, int index);

        void gid_resources_free(GidResources *gid);
        SDL_Surface *private_load_surface(const void *memres, int length);
    } // namespace Runtime
} // namespace RoninEngine
