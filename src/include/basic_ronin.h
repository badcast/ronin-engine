/*
   This is file describe type
*/

#pragma once

#include "ronin.h"

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
    // Used it matrix_key

    using namespace RoninEngine::Runtime;
    using fake_type = std::int64_t;

    template <>
    struct hash<Vec2>
    {
        std::size_t operator()(const Vec2 &val) const noexcept
        {
            return std::hash<fake_type> {}(*reinterpret_cast<const fake_type *>(&val));
        }
    };

    template <>
    struct hash<Vec2Int>
    {
        std::size_t operator()(const Vec2Int &val) const noexcept
        {
            return std::hash<fake_type> {}(*reinterpret_cast<const fake_type *>(&val));
        }
    };

} // namespace std

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

void check_object(RoninEngine::Runtime::Object *obj);

// bool matrix_compare_layer(RoninEngine::Runtime::Transform const *lhs, RoninEngine::Runtime::Transform const *rhs);

// using MatrixLayerComparer = std::integral_constant<decltype(&matrix_compare_layer), &matrix_compare_layer>;

typedef RoninEngine::Runtime::Vec2Int MatrixKey;
typedef std::map<int, std::unordered_map<MatrixKey, std::set<RoninEngine::Runtime::Transform *>>> matrix_map_t;

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

        enum : resource_id
        {
            RES_INVALID = 0xffffffff,
            RES_LOCAL_FLAG = 0x80000000
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
            int culled;
            std::set<Renderer *> prev;
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
            std::vector<void *> gid_privates;
        };

        struct AssetRef
        {
            std::unordered_map<std::size_t, resource_id> image_clips;
            std::unordered_map<std::size_t, resource_id> audio_clips;
            std::unordered_map<std::size_t, resource_id> music_clips;
        };

        struct WorldResources
        {
            // object instanced
            int objects;

            // state is unloading
            bool request_unloading;

            std::list<Sprite *> offload_sprites;
            std::list<SDL_Surface *> offload_surfaces;

            SDL_Texture *legacy_font_normal;
            SDL_Texture *legacy_font_hover;

            int audio_channels;
            int audio_reserved_channels = MIX_CHANNELS;

            // destroyed queue object
            int _destroyed;

            std::list<Behaviour *> *_firstRunScripts;
            std::list<Behaviour *> *runtimeScripts;

            // destruction task (queue object)
            std::map<float, std::set<GameObject *>> *runtimeCollectors;

            // Matrix
            matrix_map_t matrix;

            std::list<CameraResource *> camera_resources;

            // External resources
            GidResources *external_local_resources;

            // Main UI Object
            UI::GUI *gui;

            // Main camera for render
            Camera *main_camera;

            // Main or Root object
            GameObject *main_object;

            void event_camera_changed(Camera *target, CameraEvent state);
        };

        extern World *switched_world;
        extern float internal_game_time;
        extern std::list<Asset> loaded_assets;

        bool object_instanced(const Object *obj);
        void native_render_2D(Camera2D *camera);

        void harakiri_Component(Component *candidate);

        void harakiri_GameObject(GameObject *obj);

        void RuntimeCollector();
        void level_render_world();
        void level_render_world_late();
        void internal_bind_script(Behaviour *);

        void internal_load_world(World *);
        bool internal_unload_world(World *);
        void internal_free_loaded_assets();

        void hierarchy_childs_move(Transform *oldParent, Transform *newParent);
        void hierarchy_parent_change(Transform *from, Transform *newParent);
        void hierarchy_child_remove(Transform *parent, Transform *who);
        void hierarchy_parent_remove(Transform *parent);
        void hierarchy_append(Transform *parent, Transform *who);
        void hierarchy_sibiling(Transform *parent, Transform *who, int index);

        void gid_resources_free(GidResources *gid);
        SDL_Surface *private_load_surface(const void *memres, int length);

        void storm_cast_eq_all(Vec2Int origin, int edges, std::function<void(const Vec2Int &)> predicate);
        void storm_cast_eq_edges(Vec2Int origin, int edges, std::function<void(const Vec2Int &)> predicate);
    } // namespace Runtime
} // namespace RoninEngine
