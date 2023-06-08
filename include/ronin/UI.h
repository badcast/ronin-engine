#pragma once
#include "begin.h"

namespace RoninEngine
{
    namespace UI
    {
        enum TextAlign { Left, Center, Right, MiddleLeft, MiddleCenter, MiddleRight, BottomLeft, BottomCenter, BottomRight };

#define CI

        enum { NOPARENT = 0 };

        /// Тип идентификатора GUI
        typedef std::uint8_t uid;

        enum TextRandomizer_Format { All = -1, OnlyText = 0, OnlyNumber = 1, OnlyReal = 3, MaskText = 2, MaskNumber = 4, MaskReal = 8, MaskUpperChar = 16, MaskLowwerChar = 32 };

        struct {
            Runtime::Vec2Int dropdownSize = Runtime::Vec2Int(240, 30);
            Runtime::Vec2Int buttonSize = Runtime::Vec2Int(240, 30);
            Runtime::Vec2Int editSize = Runtime::Vec2Int(240, 30);
            Runtime::Vec2Int sliderSize = Runtime::Vec2Int(240, 30);
        } const defaultMakets;

        class RONIN_API GUI final
        {
        private:
            struct GUIResources* resources;

            /// Register ui element and get unique ID
            CI std::list<uid> get_groups();
            CI friend uid call_register_ui(GUI* gui, uid parent);
            CI friend UIElement& call_get_element(GUI* gui, uid id);
            CI UIElement& getElement(uid id);
            CI bool has_action(void* outPos);

        public:

            CI GUI(Runtime::World*);
            CI virtual ~GUI();

            CI bool has_ui(uid id);

            CI uid push_group(const RoninEngine::Runtime::Rect& rect);
            CI uid push_group();

            CI uid push_label(const std::string& text, const RoninEngine::Runtime::Rect& rect, const int& fontWidth = 13, uid parent = NOPARENT);
            CI uid push_label(const std::string& text, const Runtime::Vec2Int& point, const int& fontWidth = 13, uid parent = NOPARENT);
            CI uid push_button(const std::string& text, const Runtime::Vec2Int& point, ui_callback* event_callback = nullptr, uid parent = NOPARENT);
            CI uid push_button(const std::string& text, const Runtime::Rect& point, ui_callback* event_callback = nullptr, uid parent = NOPARENT);
            CI uid push_edit(const std::string& text, const Runtime::Vec2Int& point, uid parent = NOPARENT);
            CI uid push_edit(const std::string& text, const Runtime::Rect& rect, uid parent = NOPARENT);
            CI uid push_display_randomizer(TextRandomizer_Format format, const Vec2Int& point, uid parent = NOPARENT);
            CI uid push_display_randomizer(TextRandomizer_Format format = TextRandomizer_Format::All, uid parent = NOPARENT);
            CI uid push_display_randomizer_text(const std::string& text, const Vec2Int& point, uid parent = NOPARENT);
            CI uid push_display_randomizer_number(const int min, const int max, TextAlign textAlign, uid parent = NOPARENT);
            CI uid push_texture_stick(Texture* texture, const Runtime::Rect& rect, uid parent = NOPARENT);
            CI uid push_texture_stick(Texture* texture, const Runtime::Vec2Int& point, uid parent = NOPARENT);
            CI uid push_texture_animator(Timeline* timeline, const RoninEngine::Runtime::Rect& rect, uid parent = NOPARENT);
            CI uid push_texture_animator(Timeline* timeline, const Vec2Int& point, uid parent = NOPARENT);
            CI uid push_texture_animator(const std::list<Texture*>& roads, float duration, TimelineOptions option, const RoninEngine::Runtime::Rect& rect, uid parent = NOPARENT);
            CI uid push_texture_animator(const std::list<Texture*>& roads, float duration, TimelineOptions option, const Vec2Int& point, uid parent = NOPARENT);

            template <typename Container>
            CI uid push_drop_down(const Container& elements, const Runtime::Vec2Int& point, event_index_changed* changed = nullptr, uid parent = NOPARENT)
            {
                return push_drop_down(elements, 0, point, changed, parent);
            }
            template <typename Container>
            CI uid push_drop_down(const Container& elements, int index, const Runtime::Vec2Int& point, event_index_changed* changed = nullptr, uid parent = NOPARENT)
            {
                return push_drop_down(elements, index, Runtime::Rect(point.x, point.y, defaultMakets.dropdownSize.x, defaultMakets.dropdownSize.y), changed, parent);
            }
            CI uid push_drop_down(const std::vector<int>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed, uid parent);
            CI uid push_drop_down(const std::vector<float>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed, uid parent);
            CI uid push_drop_down(const std::vector<std::string>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed, uid parent);
            CI uid push_drop_down(const std::list<float>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed, uid parent);
            CI uid push_drop_down(const std::list<int>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed, uid parent);
            CI uid push_drop_down(const std::list<std::string>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed, uid parent);

            CI uid push_slider(float value, const Runtime::Vec2Int& point, event_value_changed* changed = nullptr, uid parent = NOPARENT) { return push_slider(value, 0.f, 1.f, point, changed, parent); }

            CI uid push_slider(float value, float min, float max, const Runtime::Vec2Int& point, event_value_changed* changed = nullptr, uid parent = NOPARENT)
            {
                return push_slider(value, min, max, Runtime::Rect(point.x, point.y, defaultMakets.sliderSize.x, defaultMakets.sliderSize.y), changed, parent);
            }
            CI uid push_slider(float value, float min, float max, const Runtime::Rect& rect, event_value_changed* changed = nullptr, uid parent = NOPARENT);

            // property-----------------------------------------------------------------------------------------------------------

            CI void* get_resources(uid id);
            CI void set_resources(uid id, void* data);

            CI RoninEngine::Runtime::Rect get_rect(uid id);
            CI void set_rect(uid id, const RoninEngine::Runtime::Rect& rect);

            CI std::string get_text(uid id);
            CI void set_text(uid id, const std::string& text);

            CI void set_visible(uid id, bool state);
            CI bool get_visible(uid id);

            CI void set_enable(uid id, bool state);
            CI bool get_enable(uid id);

            // grouping-----------------------------------------------------------------------------------------------------------

            CI bool is_group(uid id);
            CI void show_group_unique(uid id) throw();
            CI void show_group(uid id) throw();
            CI bool close_group(uid id) throw();

            // other--------------------------------------------------------------------------------------------------------------

            CI void set_cast(bool state);
            CI bool get_cast();

            CI void register_callback(ui_callback callback, void* userData);

            CI bool pop_element(uid id);
            CI void remove_all();

            CI void native_draw_render(SDL_Renderer* renderer);

            CI void set_color_rgb(std::uint32_t rgb);
            CI void set_color_rgba(std::uint32_t argb);

            CI bool has_focused_ui();
        };
#undef CI
    } // namespace RoninEngine::UI
}
