/*
   This is file describe type
*/

#pragma once

#include "ronin.h"
#include "ronin_pimpl.h"

#ifndef NDEBUG
#define TEST_MALLOC 0
#endif

//#define BASE_VIRTUAL_OVERRIDED(BASE, POINTER, METHOD) \
//    (reinterpret_cast<const void *>(&BASE::METHOD) == reinterpret_cast<const void *>(&((*POINTER).METHOD)))

namespace std
{
    template <>
    struct hash<RoninEngine::Runtime::Vec2>
    {
        std::size_t operator()(const RoninEngine::Runtime::Vec2 &val) const noexcept
        {
            return std::hash<std::int64_t> {}(*reinterpret_cast<const std::int64_t *>(&val));
        }
    };

    template <>
    struct hash<RoninEngine::Runtime::Vec2Int>
    {
        std::size_t operator()(const RoninEngine::Runtime::Vec2Int &val) const noexcept
        {
            return std::hash<std::int64_t> {}(*reinterpret_cast<const std::int64_t *>(&val));
        }
    };

} // namespace std

#define RONIN_USE_TYPESTR 0

#if RONIN_USE_TYPESTR

#define DESCRIBE_TYPE(TYPE, self, _type_, name) (runtime_define_type<TYPE>(#TYPE, self, (_type_), name))

static std::unordered_map<std::type_index, const char *> main_ronin_types;

// this method for replacing C++ dynamic_cast
template <typename T>
static std::enable_if_t<std::is_base_of<RoninEngine::Runtime::Object, T>::value, const char *> runtime_define_type(const char *typestr, T *self, char **_type_, const char *name)
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
#else

#define DESCRIBE_TYPE(TYPE, self, _type_, name) (#TYPE)

#endif

#define DESCRIBE_AS_ONLY_NAME(TYPE) (DESCRIBE_TYPE(TYPE, this, &_type_, name.c_str()))

#define DESCRIBE_AS_MAIN_OFF(TYPE) (DESCRIBE_TYPE(TYPE, this, &_type_, nullptr))

#define DESCRIBE_AS_MAIN(TYPE) (DESCRIBE_TYPE(TYPE, this, &_type_, name.c_str()))

// bool matrix_compare_layer(RoninEngine::Runtime::Transform const *lhs, RoninEngine::Runtime::Transform const *rhs);

// using MatrixLayerComparer = std::integral_constant<decltype(&matrix_compare_layer), &matrix_compare_layer>;

namespace RoninEngine
{
    namespace UI
    {
        extern void Render_String_ttf(const char *text, int fontSize, const Runtime::Vec2Int &screenPoint, bool alignCenter = false, bool blend = true);
    } // namespace UI

    namespace Runtime::Matrix
    {
        typedef RoninEngine::Runtime::Vec2Int matrix_key_t;
        typedef std::map<int, std::unordered_map<matrix_key_t, std::set<RoninEngine::Runtime::Transform *>>> matrix_map_t;
    } // namespace Runtime::Matrix

    namespace Runtime
    {
        enum CameraEvent
        {
            CAM_DELETED,
            CAM_TARGET
        };

        enum
        {
            RES_INVALID = 0xffffffff,
            RES_LOCAL_FLAG = 0x80000000
        };

        enum InputStateFlags
        {
            KeyDown = 1,
            KeyUp = 2
        };

        enum RenderCommand
        {
            PreRender,
            PostRender
        };

        enum RefClassType
        {
            Null = 0,
            Shared,
            Const
        };

        struct ParticleSystemImpl;

        struct RoninInput
        {
            int _mouse_wheels;
            std::int8_t _mouse_state[SDL_BUTTON_X2];
            Vec2Int _mouse_position;
            Vec2 _movement_axis;
            std::uint8_t *prev_frame_keys;
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
            Vec2 offsetScaling;
            Vec2 scale;
            std::set<Renderer *> prevs;
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

        struct font2d_t
        {
            int compressed;
            SDL_Surface *surfNormal;
            SDL_Surface *surfHilight;
            Runtime::Vec2Int fontSize;
            Runtime::Rect data[255];
        };

        struct AudioSourceData
        {
            AudioClip *m_clip;
            int targetChannel;
        };

        struct MusicPlayerData
        {
            MusicClip *m_clip;
            int loops;
        };

        struct GroupResources
        {
            std::vector<AudioClip *> gid_audio_clips;
            std::vector<MusicClip *> gid_music_clips;
            std::vector<SDL_Surface *> gid_surfaces;
            std::vector<void *> gid_privates;
        };

        enum AssetIndex
        {
            AS_SPRITE,
            AS_AUDIO,
            AS_MUSIC,
            AS_ATLAS
        };

        struct AssetImpl
        {
            int index;
            union
            {
                std::unordered_map<std::size_t, std::vector<ResId>> *_audioclips;
                std::unordered_map<std::size_t, std::vector<ResId>> *_mus;
                std::unordered_map<std::size_t, std::vector<SpriteRef>> *_sprites;
            } udata;
            AtlasRef _atlas;
        };

        struct WorldResources
        {
            // object instanced
            int objects;

            // state is unloading
            bool requestUnloading;

            std::list<SDL_Surface *> preloadeSurfaces;

            SDL_Texture *legacy_font_normal;
            SDL_Texture *legacy_font_hover;

            std::vector<bool> audioChannels;

            Vec2 metricPixelsPerPoint;

            // destroyed queue object
            int _destroyedGameObject;

            // Collection of Refs
            std::unordered_map<RoninPointer*, Ref<RoninPointer>> refPointers;

            // Script Behaviours
            std::map<GameObject::BindType, std::set<Behaviour*>> runtimeScriptBinders;

            // destruction task (queue object)
            std::map<float, std::set<GameObject*>> *runtimeCollectors;

            // Matrix
            Matrix::matrix_map_t matrix;

            std::list<CameraResource *> cameraResources;

            // External resources
            GroupResources externalLocalResources;

            // Main UI Object
            UI::GUI *gui;

            // Main camera for render
            CameraRef mainCamera;

            // Main or Root object
            GameObjectRef mainObject;

            std::map<SDL_Surface *, std::pair<int, SDL_Texture *>> renderCache;
            std::map<SDL_Texture *, SDL_Surface *> renderCacheRefs;

            void event_camera_changed(CameraRef target, CameraEvent state);
        };

        struct T2Data;

        extern World *currentWorld;
        extern float internal_game_time;
        extern std::list<AssetRef> loaded_assets;

        GameObjectRef create_game_object();
        GameObjectRef create_game_object(const std::string &name);

        void native_render_2D(Camera2D *camera);

        int sepuku_run();
        void sepuku_Component(ComponentRef &CND);
        void sepuku_GameObject(GameObjectRef obj, std::set<GameObject *> *input);

        template <typename T>
        int render_getclass();

        void level_render_world();

        void internal_load_world(World *);
        bool internal_unload_world(World *);
        void internal_free_loaded_assets();

        void hierarchy_childs_move(Transform *oldParent, Transform *newParent);
        void hierarchy_parent_change(TransformRef from, TransformRef newParent);
        void hierarchy_child_remove(Transform *parent, Transform *who);
        void hierarchy_childs_remove(Transform *parent);
        void hierarchy_append(TransformRef parent, TransformRef who);
        void hierarchy_sibiling(Transform *parent, Transform *who, int index);

        void scripts_start();
        void scripts_update();
        void scripts_lateUpdate();
        void scripts_gizmos();
        void scripts_unbind(BehaviourRef script);

        void gid_resources_free(GroupResources *gid);
        GroupResources *gid_get(bool local);
        SDL_Surface *private_load_surface(const void *memres, int size, bool local = false);

        void storm_cast_eq_all(Vec2Int origin, int edges, std::function<void(const Vec2Int &)> predicate);
        void storm_cast_eq_edges(Vec2Int origin, int edges, std::function<void(const Vec2Int &)> predicate);

        constexpr inline RoninPointer* RefNoFree(RoninPointer * object)
        {
            object->_handle = RefClassType::Const;
            return object;
        }
        template<typename T>
        constexpr inline Ref<T> RefNoFree(Ref<T> object)
        {
            if(object)
                RefNoFree(static_cast<RoninPointer*>(object.ptr_));
            return object;
        }
        template<typename T>
        void RefMarkNull(Ref<T> object);
        void RefReleaseSoft(RoninPointer* object);
        void RefReleaseHard(RoninPointer* object);
    } // namespace Runtime

    extern struct RoninEnvironment
    {
        struct
        {
            // this is variable for apply settings
            int conf;
            RenderDriverInfo::RenderBackend renderBackend = RenderDriverInfo::RenderBackend::GPU;
        } config;

        SDL_Renderer *renderer = nullptr;

        SDL_Window *activeWindow = nullptr;
        Resolution activeResolution {0, 0, 0};
        TimingWatcher lastWatcher {};
        TimingWatcher queueWatcher {};
        std::vector<SDL_Cursor *> sysCursors {};

        bool debugMode = false;
        bool internalWorldLoaded = false;
        bool internalWorldCanStart = false;

        Runtime::MusicPlayerData musicData;
    } gscope;

} // namespace RoninEngine
