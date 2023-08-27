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
            struct GUIResources *_resources;

        public:
            CI GUI(Runtime::World *);
            CI virtual ~GUI();

            CI bool ElementContains(uid id);

            CI uid PushGroup(const RoninEngine::Runtime::Rect &rect);
            CI uid PushGroup();

            /**
             * After layment require end_layment() for end selection layer
             *
             * \param region - the region of drawing object
             *
             * \see end_layment()
             */
            CI uid LayoutBegin(const Runtime::Rect region);
            CI void LayoutBegin();

            // CI uid push_layment_label(const std::string &text);
            // CI uid push_layment_button(const std::string &text, ui_callback_void event_callback = nullptr);
            // CI uid push_layment_edit(const std::string &text, ui_callback_chars event_callback = nullptr);
            // CI uid push_layment_slider(float value, float min = 0.f, float max = 1.f, ui_callback_float value_changed = nullptr);

            CI uid PushLabel(const std::string &text, const Runtime::Rect &rect, const int &fontWidth = 13, uid parent = NOPARENT);
            CI uid PushLabel(const std::string &text, const Runtime::Vec2Int &point, const int &fontWidth = 13, uid parent = NOPARENT);
            CI uid PushButton(
                const std::string &text, const Runtime::Vec2Int &point, ui_callback_void event_callback = nullptr, uid parent = NOPARENT);
            CI uid PushButton(
                const std::string &text, const Runtime::Rect &point, ui_callback_void event_callback = nullptr, uid parent = NOPARENT);
            CI uid PushTextEdit(const std::string &text, const Runtime::Vec2Int &point, uid parent = NOPARENT);
            CI uid PushTextEdit(const std::string &text, const Runtime::Rect &rect, uid parent = NOPARENT);
            CI uid PushPictureBox(Runtime::Sprite *sprite, const Runtime::Rect &rect, uid parent = NOPARENT);
            CI uid PushPictureBox(Runtime::Sprite *sprite, const Runtime::Vec2Int &point, uid parent = NOPARENT);

            CI uid PushDropDown(
                const std::list<std::string> &elements, int index, const Runtime::Rect &rect, ui_callback_integer changed, uid parent);

            template <typename string_container>
            CI uid PushDropDown(
                const string_container &container,
                int index,
                const Runtime::Vec2Int &point,
                ui_callback_integer changed = nullptr,
                uid parent = NOPARENT)
            {
                return PushDropDown(container, index, {point, defaultMakets.dropdownSize}, changed, parent);
            }

            template <typename string_container>
            CI uid PushDropDown(
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

                return PushDropDown(__convert_string, index, rect, changed, parent);
            }

            CI uid PushSlider(float value, const Runtime::Vec2Int &point, ui_callback_float changed = nullptr, uid parent = NOPARENT)
            {
                return PushSlider(value, 0.f, 1.f, point, changed, parent);
            }

            CI uid PushSlider(
                float value,
                float min,
                float max,
                const Runtime::Vec2Int &point,
                ui_callback_float changed = nullptr,
                uid parent = NOPARENT)
            {
                return PushSlider(
                    value,
                    min,
                    max,
                    Runtime::Rect(point.x, point.y, defaultMakets.sliderSize.x, defaultMakets.sliderSize.y),
                    changed,
                    parent);
            }
            CI uid PushSlider(
                float value, float min, float max, const Runtime::Rect &rect, ui_callback_float changed = nullptr, uid parent = NOPARENT);

            // property-----------------------------------------------------------------------------------------------------------

            CI RoninEngine::Runtime::Rect GetElementRect(uid id);
            CI void SetElementRect(uid id, const Runtime::Rect &rect);

            CI void SetElementText(uid id, const std::string &text);
            CI std::string GetElementText(uid id);

            CI void SetElementVisible(uid id, bool state);
            CI bool GetElementVisible(uid id);

            CI void SetElementEnable(uid id, bool state);
            CI bool GetElementEnable(uid id);

            CI void SliderSetValue(uid id, float value);
            CI float SliderGetValue(uid id);

            // grouping-----------------------------------------------------------------------------------------------------------

            CI bool IsGroup(uid id);
            CI void GroupShowAsUnique(uid id) throw();
            CI void GroupShow(uid id) throw();
            CI bool GroupClose(uid id) throw();

            // other--------------------------------------------------------------------------------------------------------------

            bool IsMouseOver();

            CI void SetInteractable(bool state);
            CI bool GetInteractable();

            CI void SetGeneralCallback(ui_callback callback, void *userData);

            CI bool PopElement(uid id);
            CI void PopAllElements();

            CI bool FocusedGUI();
        };
#undef CI
    } // namespace UI
} // namespace RoninEngine
