#pragma once

#include "begin.h"
#include "Controls.h"

namespace RoninEngine
{
    namespace UI
    {
        enum Align
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

#define UI_METHOD

        enum
        {
            NOPARENT = 0
        };

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

        static struct
        {
            Runtime::Vec2Int dropdownSize {240, 30};
            Runtime::Vec2Int buttonSize {240, 30};
            Runtime::Vec2Int editSize {240, 30};
            Runtime::Vec2Int sliderSize {240, 30};
            Runtime::Vec2Int checkboxSize {120, 25};
        } const defaultMakets;

        struct UIData
        {
            uid id;
            Runtime::Rect rect;
            Runtime::Vec2Int mousePoint;
            bool isMouseDown;
            bool isMouseHover;
        };

        class UIOverlay
        {
        public:
            virtual void OnInit() = 0;
            virtual void OnDraw(const UIData *const uiData) = 0;
            virtual void OnDestroy() = 0;
        };

        class RONIN_API GUI final
        {
        private:
            struct GUIResources *handle;

        public:
            UI_METHOD GUI(Runtime::World *);
            UI_METHOD ~GUI();

            UI_METHOD uid PushCustomUI(const UIOverlay *custom, const Runtime::Rect &rect, uid parent = NOPARENT);

            // Layments
            UI_METHOD void LayoutNew(Runtime::Rectf region, bool aspect = true);
            UI_METHOD uid LayoutLabel(const std::string &text);
            UI_METHOD uid LayoutButton(const std::string &text);

            UI_METHOD bool ElementContains(uid id);

            UI_METHOD uid PushGroup(const RoninEngine::Runtime::Rect &rect);
            UI_METHOD uid PushGroup();

            UI_METHOD uid PushLabel(const std::string &text, const Runtime::Rect &rect, int fontWidth = 13, uid parent = NOPARENT);
            UI_METHOD uid PushLabel(const std::string &text, const Runtime::Vec2Int &point, int fontWidth = 13, uid parent = NOPARENT);
            UI_METHOD uid
            PushButton(const std::string &text, const Runtime::Vec2Int &point, UIEventVoid event_callback = nullptr, uid parent = NOPARENT);
            UI_METHOD uid
            PushButton(const std::string &text, const Runtime::Rect &point, UIEventVoid event_callback = nullptr, uid parent = NOPARENT);
            UI_METHOD uid PushTextEdit(const std::string &text, const Runtime::Vec2Int &point, uid parent = NOPARENT);
            UI_METHOD uid PushTextEdit(const std::string &text, const Runtime::Rect &rect, uid parent = NOPARENT);
            UI_METHOD uid PushPictureBox(Runtime::Sprite *sprite, const Runtime::Rect &rect, uid parent = NOPARENT);
            UI_METHOD uid PushPictureBox(Runtime::Sprite *sprite, const Runtime::Vec2Int &point, uid parent = NOPARENT);

            UI_METHOD uid
            PushDropDown(const std::list<std::string> &elements, int index, const Runtime::Rect &rect, UIEventInteger changed, uid parent);

            template <typename string_container>
            UI_METHOD uid PushDropDown(
                const string_container &container,
                int index,
                const Runtime::Vec2Int &point,
                UIEventInteger changed = nullptr,
                uid parent = NOPARENT)
            {
                return PushDropDown(container, index, {point, defaultMakets.dropdownSize}, changed, parent);
            }

            template <typename string_container>
            UI_METHOD uid PushDropDown(
                const string_container &container,
                int index,
                const Runtime::Rect &rect,
                UIEventInteger changed = nullptr,
                uid parent = NOPARENT)
            {
                using T = typename string_container::value_type;

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

            UI_METHOD uid PushSlider(float value, const Runtime::Vec2Int &point, UIEventFloat changed = nullptr, uid parent = NOPARENT)
            {
                return PushSlider(value, 0.f, 1.f, point, changed, parent);
            }

            UI_METHOD uid PushSlider(
                float value, float min, float max, const Runtime::Vec2Int &point, UIEventFloat changed = nullptr, uid parent = NOPARENT)
            {
                return PushSlider(
                    value,
                    min,
                    max,
                    Runtime::Rect(point.x, point.y, defaultMakets.sliderSize.x, defaultMakets.sliderSize.y),
                    changed,
                    parent);
            }
            UI_METHOD uid
            PushSlider(float value, float min, float max, const Runtime::Rect &rect, UIEventFloat changed = nullptr, uid parent = NOPARENT);

            UI_METHOD uid PushCheckBox(
                bool checked, const std::string &text, const Runtime::Vec2Int &point, UIEventBool changed = nullptr, uid parent = NOPARENT)
            {
                return PushCheckBox(checked, text, Runtime::Rect {point, defaultMakets.checkboxSize}, changed, parent);
            }

            UI_METHOD uid PushCheckBox(
                bool checked, const std::string &text, const Runtime::Rect &rect, UIEventBool changed = nullptr, uid parent = NOPARENT);

            // property-----------------------------------------------------------------------------------------------------------

            UI_METHOD void ElementSetRect(uid id, const Runtime::Rect &rect);
            UI_METHOD Runtime::Rect ElementGetRect(uid id);

            UI_METHOD void ElementSetText(uid id, const std::string &text);
            UI_METHOD std::string ElementGetText(uid id);

            UI_METHOD void ElementSetVisible(uid id, bool state);
            UI_METHOD bool ElementGetVisible(uid id);

            UI_METHOD void ElementSetEnable(uid id, bool state);
            UI_METHOD bool ElementGetEnable(uid id);

            UI_METHOD void SliderSetValue(uid id, float value);
            UI_METHOD float SliderGetValue(uid id);

            UI_METHOD bool CheckBoxGetValue(uid id);
            UI_METHOD void CheckBoxSetValue(uid id, bool value);

            UI_METHOD void SliderSetPercentage(uid id, float percentage);
            UI_METHOD float SliderGetPercentage(uid id);

            UI_METHOD bool ButtonClicked(uid id);

            UI_METHOD void PictureBoxSetSprite(uid id, Runtime::Sprite *sprite);
            UI_METHOD Runtime::Sprite *PictureBoxGetSprite(uid id);

            // grouping-----------------------------------------------------------------------------------------------------------

            UI_METHOD bool IsGroup(uid id);
            UI_METHOD bool GroupShowAsUnique(uid id) throw();
            UI_METHOD bool GroupShow(uid id) throw();
            UI_METHOD bool GroupClose(uid id) throw();

            // other--------------------------------------------------------------------------------------------------------------

            UI_METHOD bool IsMouseOver();

            UI_METHOD void SetInteractable(bool state);
            UI_METHOD bool GetInteractable();

            UI_METHOD void SetGeneralCallback(UIEventUserData callback, void *userData);

            UI_METHOD bool PopElement(uid id);
            UI_METHOD void PopAllElements();

            UI_METHOD bool FocusedGUI();
        };
#undef UI_METHOD
    } // namespace UI
} // namespace RoninEngine
