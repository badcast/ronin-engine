#pragma once

#include "begin.h"
#include "Controls.h"

namespace RoninEngine
{
    namespace UI
    {
        enum TextAlign
        {
            Left,
            Center,
            Right,
            MiddleLeft,
            MiddleCenter,
            MiddleRight,
            BottomLeft,
            BottomCenter,
            BottomRight
        };

#define CI

        enum
        {
            NOPARENT = 0
        };

        /// Тип идентификатора GUI
        typedef std::uint8_t uid;

        enum TextRandomizer_Format
        {
            All = -1,
            OnlyText = 0,
            OnlyNumber = 1,
            OnlyReal = 3,
            MaskText = 2,
            MaskNumber = 4,
            MaskReal = 8,
            MaskUpperChar = 16,
            MaskLowwerChar = 32
        };

        struct
        {
            Runtime::Vec2Int dropdownSize = Runtime::Vec2Int(240, 30);
            Runtime::Vec2Int buttonSize = Runtime::Vec2Int(240, 30);
            Runtime::Vec2Int editSize = Runtime::Vec2Int(240, 30);
            Runtime::Vec2Int sliderSize = Runtime::Vec2Int(240, 30);
        } const defaultMakets;

        class RONIN_API GUI final
        {
        private:
            struct GUIResources *resources;

        public:
            CI GUI(Runtime::World *);
            CI virtual ~GUI();

            CI bool has_ui(uid id);

            CI uid push_group(const RoninEngine::Runtime::Rect &rect);
            CI uid push_group();

            /**
             * After layment require end_layment() for end selection layer
             *
             * \param region - the region of drawing object
             *
             * \see end_layment()
             */
            CI uid begin_layment(const Runtime::Rect region);
            CI void end_layment();

            CI uid push_layment_label(const std::string &text);
            CI uid push_layment_button(const std::string &text, ui_callback_void event_callback = nullptr);
            CI uid push_layment_edit(const std::string &text, ui_callback_chars event_callback = nullptr);
            CI uid push_layment_slider(
                float value, float min = 0.f, float max = 1.f, ui_callback_float value_changed = nullptr);

            CI uid push_label(
                const std::string &text, const Runtime::Rect &rect, const int &fontWidth = 13, uid parent = NOPARENT);
            CI uid push_label(
                const std::string &text,
                const Runtime::Vec2Int &point,
                const int &fontWidth = 13,
                uid parent = NOPARENT);
            CI uid push_button(
                const std::string &text,
                const Runtime::Vec2Int &point,
                ui_callback_void event_callback = nullptr,
                uid parent = NOPARENT);
            CI uid push_button(
                const std::string &text,
                const Runtime::Rect &point,
                ui_callback_void event_callback = nullptr,
                uid parent = NOPARENT);
            CI uid push_edit(const std::string &text, const Runtime::Vec2Int &point, uid parent = NOPARENT);
            CI uid push_edit(const std::string &text, const Runtime::Rect &rect, uid parent = NOPARENT);
            CI uid push_picture(Runtime::Sprite *sprite, const Runtime::Rect &rect, uid parent = NOPARENT);
            CI uid push_picture(Runtime::Sprite *sprite, const Runtime::Vec2Int &point, uid parent = NOPARENT);

            CI uid push_drop_down(
                const std::list<std::string> &elements,
                int index,
                const Runtime::Rect &rect,
                ui_callback_integer changed,
                uid parent);

            template <typename string_container>
            CI uid push_drop_down(
                const string_container &container,
                int index,
                const Runtime::Vec2Int &point,
                ui_callback_integer changed = nullptr,
                uid parent = NOPARENT)
            {
                return push_drop_down(container, index, {point, defaultMakets.dropdownSize}, changed, parent);
            }

            template <typename string_container>
            CI uid push_drop_down(
                const string_container &container,
                int index,
                const Runtime::Rect &rect,
                ui_callback_integer changed = nullptr,
                uid parent = NOPARENT)
            {
                typedef typename string_container::value_type T;

                std::list<std::string> __convert_string;

                for(auto iter = std::begin(container); iter != std::end(container); ++iter)
                {
                    if constexpr(std::is_same<std::string, T>::value)
                        __convert_string.emplace_back(*iter);
                    else if constexpr(std::is_same<const char *, T>::value)
                        __convert_string.emplace_back(*iter);
                    else
                        __convert_string.emplace_back(std::to_string(*iter));
                }

                return push_drop_down(__convert_string, index, rect, changed, parent);
            }

            CI uid push_slider(
                float value, const Runtime::Vec2Int &point, ui_callback_float changed = nullptr, uid parent = NOPARENT)
            {
                return push_slider(value, 0.f, 1.f, point, changed, parent);
            }

            CI uid push_slider(
                float value,
                float min,
                float max,
                const Runtime::Vec2Int &point,
                ui_callback_float changed = nullptr,
                uid parent = NOPARENT)
            {
                return push_slider(
                    value,
                    min,
                    max,
                    Runtime::Rect(point.x, point.y, defaultMakets.sliderSize.x, defaultMakets.sliderSize.y),
                    changed,
                    parent);
            }
            CI uid push_slider(
                float value,
                float min,
                float max,
                const Runtime::Rect &rect,
                ui_callback_float changed = nullptr,
                uid parent = NOPARENT);

            // property-----------------------------------------------------------------------------------------------------------

            CI void *get_resources(uid id);
            CI void set_resources(uid id, void *data);

            CI RoninEngine::Runtime::Rect get_rect(uid id);
            CI void set_rect(uid id, const Runtime::Rect &rect);

            CI std::string get_text(uid id);
            CI void set_text(uid id, const std::string &text);

            CI void set_visible(uid id, bool state);
            CI bool get_visible(uid id);

            CI void set_enable(uid id, bool state);
            CI bool get_enable(uid id);

            CI float get_slider_value(uid id);
            CI void set_slider_value(uid id, float value);

            // grouping-----------------------------------------------------------------------------------------------------------

            CI bool is_group(uid id);
            CI void show_group_unique(uid id) throw();
            CI void show_group(uid id) throw();
            CI bool close_group(uid id) throw();

            // other--------------------------------------------------------------------------------------------------------------

            CI void set_cast(bool state);
            CI bool get_cast();

            CI void register_general_callback(ui_callback callback, void *userData);

            CI bool pop_element(uid id);
            CI void remove_all();

            CI void set_color_rgb(std::uint32_t rgb);
            CI void set_color_rgba(std::uint32_t argb);

            CI bool has_focused_ui();
        };
#undef CI
    } // namespace UI
} // namespace RoninEngine
