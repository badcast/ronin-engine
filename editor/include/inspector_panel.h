#pragma once

#include "editor_state.h"
#include <functional>
#include <filesystem>
#include "miniaudio/miniaudio.h"
struct SDL_Renderer;
struct SDL_Texture;

namespace RoninEditor
{
    enum class ComponentType
    {
        Transform,
        SpriteRenderer,
        Camera2D,
        Light2D,
        Terrain2D,
        ParticleSystem,
        PhysicsController,
        Rigidbody2D,
        BoxCollider2D,
        MultiCollider2D,
        NavMeshAgent,
        AudioSource,
        MotionScript
    };

    struct ComponentClipboard
    {
        bool hasData = false;
        ComponentType type = ComponentType::Transform;

        // Transform
        RoninEngine::Runtime::Vec2 position{0.0f, 0.0f};
        float angle = 0.0f;
        RoninEngine::Runtime::Vec2 scale{1.0f, 1.0f};

        // Components
        SpriteRendererData sprite;
        Camera2DData camera;
        Light2DData light;
        Terrain2DData terrain;
        ParticleSystemData particles;
        PhysicsControllerData physics;
        Rigidbody2DData rigidbody2D;
        BoxCollider2DData boxCollider2D;
        MultiCollider2DData multiCollider2D;
        NavMeshAgentData navMeshAgent;
        AudioSourceData audio;
        MotionScriptData script;
    };

    struct ComponentHeaderResult
    {
        bool isOpen = false;
        bool reset = false;
        bool remove = false;
        bool copy = false;
        bool paste = false;
        bool duplicate = false;
    };

    struct WaveformData
    {
        std::filesystem::path path;
        std::filesystem::file_time_type lastWriteTime{};
        std::vector<float> peaks; // 128 normalized amplitude bars
        double duration = 0.0;
        uint32_t sampleRate = 0;
        uint32_t channels = 0;
        uint64_t totalFrames = 0;
        bool valid = false;
    };

    class InspectorPanel
    {
    public:
        InspectorPanel();
        ~InspectorPanel();

        void Init(::SDL_Renderer *renderer);
        void Render(EditorState &state);

        std::function<void(const std::filesystem::path &)> onOpenSpriteEditor;
        std::function<void(const std::filesystem::path &)> onPingAsset;

    private:
        void renderAssetInspector(EditorState &state);
        ComponentHeaderResult renderComponentHeader(
            const char *title,
            int componentId,
            ComponentType type,
            bool allowRemove = true,
            bool defaultOpen = true);

        void duplicateComponentToNewObject(EditorState &state, EditorObject &obj, ComponentType type, const char *title);

        void renderTransformComponent(EditorState &state, EditorObject &obj);
        void renderSpriteRendererComponent(EditorState &state, EditorObject &obj);
        void renderCamera2DComponent(EditorState &state, EditorObject &obj);
        void renderLight2DComponent(EditorState &state, EditorObject &obj);
        void renderTerrain2DComponent(EditorState &state, EditorObject &obj);
        void renderParticleSystemComponent(EditorState &state, EditorObject &obj);
        void renderPhysicsComponent(EditorState &state, EditorObject &obj);
        void renderRigidbody2DComponent(EditorState &state, EditorObject &obj);
        void renderBoxCollider2DComponent(EditorState &state, EditorObject &obj);
        void renderMultiCollider2DComponent(EditorState &state, EditorObject &obj);
        void renderNavMeshAgentComponent(EditorState &state, EditorObject &obj);
        void renderAudioSourceComponent(EditorState &state, EditorObject &obj);
        void renderMotionScriptComponent(EditorState &state, EditorObject &obj);
        void renderAddComponentMenu(EditorState &state, EditorObject &obj);

        void renderSpritePickerModal(EditorObject &obj, EditorState &state);
        void renderAudioPickerModal(EditorObject &obj, EditorState &state);

        ma_engine *getAudioEngine();
        void playAudioPreview(const std::string &clipPath);
        void pauseAudioPreview();
        void stopAudioPreview();
        void seekAudioPreview(double timeSeconds);
        void ensureWaveformCached(const std::filesystem::path &p);
        void ensureTextureLoaded(const std::filesystem::path &p);

        // Clipboard
        ComponentClipboard m_clipboard;

        // Modals
        bool m_showSpritePicker = false;
        bool m_showAudioPicker = false;
        bool m_openSpritePickerTrigger = false;
        bool m_openAudioPickerTrigger = false;
        std::string m_pendingSpritePath;
        uint32_t m_pendingSpriteAssetId = 0;
        std::string m_pendingAudioPath;
        uint32_t m_pendingAudioAssetId = 0;
        char m_pickerFilter[128] = "";

        // Renderer & Texture preview
        ::SDL_Renderer *m_renderer = nullptr;
        ::SDL_Texture *m_previewTexture = nullptr;
        std::filesystem::path m_previewTexturePath;
        int m_previewTexWidth = 0;
        int m_previewTexHeight = 0;

        // Audio preview playback via miniaudio & waveform player
        ma_engine m_editorAudioEngine;
        bool m_audioEngineInitialized = false;
        ma_sound m_previewSound;
        bool m_previewSoundActive = false;
        bool m_previewPaused = false;
        bool m_previewLooping = false;
        std::string m_previewPlayingPath;
        float m_previewVolume = 0.8f;

        WaveformData m_cachedWaveform;
    };
} // namespace RoninEditor
