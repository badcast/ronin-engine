/*
   This is file describe type
*/

#pragma once

#include "ronin.h"

#ifndef NDEBUG
#define TEST_MALLOC 0
#endif

//#define BASE_VIRTUAL_OVERRIDED(BASE, POINTER, METHOD) \
//    (reinterpret_cast<const void *>(&BASE::METHOD) == reinterpret_cast<const void *>(&((*POINTER).METHOD)))

namespace std
{
    // this for Hashtable function
    // Used it matrix_key
    using fake_type = std::int64_t;

    template <>
    struct hash<RoninEngine::Runtime::Vec2>
    {
        std::size_t operator()(const RoninEngine::Runtime::Vec2 &val) const noexcept
        {
            return std::hash<fake_type> {}(*reinterpret_cast<const fake_type *>(&val));
        }
    };

    template <>
    struct hash<RoninEngine::Runtime::Vec2Int>
    {
        std::size_t operator()(const RoninEngine::Runtime::Vec2Int &val) const noexcept
        {
            return std::hash<fake_type> {}(*reinterpret_cast<const fake_type *>(&val));
        }
    };

} // namespace std

#define RONIN_USE_TYPESTR 0

#if RONIN_USE_TYPESTR

#define DESCRIBE_TYPE(TYPE, self, _type_, name) (runtime_define_type<TYPE>(#TYPE, self, (_type_), name))

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
#else

#define DESCRIBE_TYPE(TYPE, self, _type_, name) (#TYPE)

#endif

#define DESCRIBE_AS_ONLY_NAME(TYPE) (DESCRIBE_TYPE(TYPE, this, &_type_, name.c_str()))

#define DESCRIBE_AS_MAIN_OFF(TYPE) (DESCRIBE_TYPE(TYPE, this, &_type_, nullptr))

#define DESCRIBE_AS_MAIN(TYPE) (DESCRIBE_TYPE(TYPE, this, &_type_, name.c_str()))

// bool matrix_compare_layer(RoninEngine::Runtime::Transform const *lhs, RoninEngine::Runtime::Transform const *rhs);

// using MatrixLayerComparer = std::integral_constant<decltype(&matrix_compare_layer), &matrix_compare_layer>;

typedef RoninEngine::Runtime::Vec2Int matrix_key_t;
typedef std::map<int, std::unordered_map<matrix_key_t, std::set<RoninEngine::Runtime::Transform *>>> matrix_map_t;
namespace RoninEngine
{
    namespace UI
    {
        struct LegacyFont_t
        {
            int compressed;
            SDL_Surface *surfNormal;
            SDL_Surface *surfHilight;
            Runtime::Vec2Int fontSize;
            Runtime::Rect data[255];
        };

        extern void Render_String_ttf(
            const char *text, int fontSize, const Runtime::Vec2Int &screenPoint, bool alignCenter = false, bool blend = true);
    } // namespace UI

    namespace Runtime
    {
        enum class CameraEvent
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

        enum class RenderCommand
        {
            PreRender,
            PostRender
        };

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

        struct AudioSourceData
        {
            AudioClip *m_clip;
            int target_channel;
        };

        struct MusicPlayerData
        {
            MusicClip *m_clip;
            int loops;
        };

        struct GidResources
        {
            std::vector<AudioClip *> gid_audio_clips;
            std::vector<MusicClip *> gid_music_clips;
            std::vector<SDL_Surface *> gid_surfaces;
            std::vector<Sprite *> gid_sprites;
            std::vector<void *> gid_privates;
        };

        struct AssetRef
        {
            std::unordered_map<std::size_t, ResId> bindAudioClips;
            std::unordered_map<std::size_t, ResId> bindMusicClips;
            std::unordered_map<std::size_t, Sprite*> bindSprites;

            Atlas *atlasRef;
        };

        struct ParticleDrain
        {
            SpriteRenderer *render;
            float initTime;
            Vec2 direction;
        };

        struct ParticleSystemRef
        {
            // TODO: Optimize here for Particle member (Contain in to ronin_particles*.cpp
            std::set<ParticleDrain> activeParticles;
            std::vector<SpriteRenderer *> cachedParticles;
            float m_timing;
            int m_maked;
            int m_lastInspected = 0;
            int m_limit = 0;

            // bool:colorable - if true simulate it
            bool colorable = true;
            Color startColor = Color::transparent;
            Color centerColor = Color::white;
            Color endColor = Color::transparent;

            // bool:scalable - if true simulate it
            bool scalable = true;
            Vec2 startSize = Vec2::zero;
            Vec2 centerSize = Vec2::one;
            Vec2 endSize = Vec2::zero;

            // Lifetime for particle
            float m_duration = 10;
            float m_durationStartRange = 0.1f; // Range [0.0,1.0]
            float m_durationEndRange = 0.1f;   // Range [0.0,1.0]

            void link_particles(ParticleSystem *from, int n);
            void unlink_particle(ParticleSystem *from, const ParticleDrain *drain);
        };

        struct WorldResources
        {
            // object instanced
            int objects;

            // state is unloading
            bool request_unloading;

            std::list<SDL_Surface *> offload_surfaces;

            SDL_Texture *legacy_font_normal;
            SDL_Texture *legacy_font_hover;

            std::vector<bool> audioChannels;

            Vec2 metricPixelsPerPoint;

            // destroyed queue object
            int _destroyedGameObject;

            // Script Behaviours
            std::map<GameObject::BindType, std::set<Behaviour *>> runtimeScriptBinders;

            // destruction task (queue object)
            std::map<float, std::set<GameObject *>> *runtimeCollectors;

            // Matrix
            matrix_map_t matrix;

            std::list<CameraResource *> cameraResources;

            // External resources
            GidResources externalLocalResources;

            // Main UI Object
            UI::GUI *gui;

            // Main camera for render
            Camera *mainCamera;

            // Main or Root object
            GameObject *mainObject;

            std::map<SDL_Surface *, std::pair<int, SDL_Texture *>> renderCache;
            std::map<SDL_Texture *, SDL_Surface *> renderCacheRefs;

            void event_camera_changed(Camera *target, CameraEvent state);
        };

        /*** PIMPL (pointer to implementation) ***/
        class ParticleSystemRef;

        extern World *switched_world;
        extern float internal_game_time;
        extern std::list<Asset> loaded_assets;

        // using LowerParticleDrain = std::integral_constant<decltype(&IsLowerParticleDrain), &IsLowerParticleDrain>;
        bool operator<(const ParticleDrain &lhs, const ParticleDrain &rhs);

        GameObject *create_game_object();
        GameObject *create_game_object(const std::string &name);

        bool object_instanced(const Object *obj);
        void native_render_2D(Camera2D *camera);

        void harakiri_Component(Component *candidate);

        void harakiri_GameObject(GameObject *obj, std::set<GameObject *> *input);

        void Bushido_Tradition_Harakiri();

        void level_render_world();

        void internal_load_world(World *);
        bool internal_unload_world(World *);
        void internal_free_loaded_assets();

        void hierarchy_childs_move(Transform *oldParent, Transform *newParent);
        void hierarchy_parent_change(Transform *from, Transform *newParent);
        void hierarchy_child_remove(Transform *parent, Transform *who);
        void hierarchy_childs_remove(Transform *parent);
        void hierarchy_append(Transform *parent, Transform *who);
        void hierarchy_sibiling(Transform *parent, Transform *who, int index);

        void scripts_start();
        void scripts_update();
        void scripts_lateUpdate();
        void scripts_gizmos();
        void scripts_unbind(Behaviour *script);

        void gid_resources_free(GidResources *gid);
        GidResources *gid_get(bool local);
        SDL_Surface *private_load_surface(const void *memres, int length);

        void storm_cast_eq_all(Vec2Int origin, int edges, std::function<void(const Vec2Int &)> predicate);
        void storm_cast_eq_edges(Vec2Int origin, int edges, std::function<void(const Vec2Int &)> predicate);
    } // namespace Runtime

    extern struct RoninEnvironment
    {
        struct
        {
            // this is variable for apply settings
            int conf;
            RenderDriverInfo::RenderBackend renderBackend = RenderDriverInfo::RenderBackend::GPU;
        } simConfig;

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
