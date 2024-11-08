#pragma once

#include "Controls.h"
#include "begin.h"

namespace RoninEngine
{
    namespace UI
    {

#define UI

        /**
         * @brief The UID not valid ID
         */
        enum
        {
            NOPARENT = 0
        };

        enum UILayoutDirection
        {
            Vertical,
            Horizontal
        };

        enum UIAlign
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
            UI GUI(Runtime::World *);
            UI ~GUI();

            UI uid PushCustomUI(const UIOverlay *custom, const Runtime::Rect &rect, uid parent = NOPARENT);

                   // Layments
            UI void LayoutNew(UILayoutDirection direction, Runtime::Rectf region = Runtime::Rectf {.0f, .0f, 1.0f, 1.0f}, bool aspectRatio = true);

            UI uid LayoutLabel(const std::string &text);
            UI uid LayoutButton(const std::string &text);

            UI bool ElementContains(uid id);

            UI uid PushGroup(const RoninEngine::Runtime::Rect &rect);
            UI uid PushGroup();

            UI uid PushLabel(const std::string &text, const Runtime::Rect &rect, int fontWidth = 13, uid parent = NOPARENT);
            UI uid PushLabel(const std::string &text, const Runtime::Vec2Int &point, int fontWidth = 13, uid parent = NOPARENT);
            UI uid PushButton(const std::string &text, const Runtime::Vec2Int &point, UIEventVoid event_callback = nullptr, uid parent = NOPARENT);
            UI uid PushButton(const std::string &text, const Runtime::Rect &point, UIEventVoid event_callback = nullptr, uid parent = NOPARENT);
            UI uid PushTextEdit(const std::string &text, const Runtime::Vec2Int &point, uid parent = NOPARENT);
            UI uid PushTextEdit(const std::string &text, const Runtime::Rect &rect, uid parent = NOPARENT);
            UI uid PushPictureBox(Runtime::SpriteRef sprite, const Runtime::Rect &rect, uid parent = NOPARENT);
            UI uid PushPictureBox(Runtime::SpriteRef sprite, const Runtime::Vec2Int &point, uid parent = NOPARENT);

            UI uid PushDropDown(const std::list<std::string> &elements, int index, const Runtime::Rect &rect, UIEventInteger changed, uid parent);

            template <typename string_container>
            UI uid PushDropDown(const string_container &container, int index, const Runtime::Vec2Int &point, UIEventInteger changed = nullptr, uid parent = NOPARENT)
            {
                return PushDropDown(container, index, {point, defaultMakets.dropdownSize}, changed, parent);
            }

            template <typename string_container>
            UI uid PushDropDown(const string_container &container, int index, const Runtime::Rect &rect, UIEventInteger changed = nullptr, uid parent = NOPARENT)
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

            UI uid PushSlider(float value, const Runtime::Vec2Int &point, UIEventFloat changed = nullptr, uid parent = NOPARENT)
            {
                return PushSlider(value, 0.f, 1.f, point, changed, parent);
            }

            UI uid PushSlider(float value, float min, float max, const Runtime::Vec2Int &point, UIEventFloat changed = nullptr, uid parent = NOPARENT)
            {
                return PushSlider(value, min, max, Runtime::Rect(point.x, point.y, defaultMakets.sliderSize.x, defaultMakets.sliderSize.y), changed, parent);
            }
            UI uid PushSlider(float value, float min, float max, const Runtime::Rect &rect, UIEventFloat changed = nullptr, uid parent = NOPARENT);

            UI uid PushCheckBox(bool checked, const std::string &text, const Runtime::Vec2Int &point, UIEventBool changed = nullptr, uid parent = NOPARENT)
            {
                return PushCheckBox(checked, text, Runtime::Rect {point, defaultMakets.checkboxSize}, changed, parent);
            }

            UI uid PushCheckBox(bool checked, const std::string &text, const Runtime::Rect &rect, UIEventBool changed = nullptr, uid parent = NOPARENT);

            UI uid PushSpriteButton(const std::vector<Runtime::SpriteRef > &states, const Runtime::Rect &rect, UIEventVoid click = nullptr, uid parent = NOPARENT);

            UI void SpriteButtonSetIcon(uid id, Runtime::SpriteRef icon);

                   // property-----------------------------------------------------------------------------------------------------------

            UI void ElementSetRect(uid id, const Runtime::Rect &rect);
            UI Runtime::Rect ElementGetRect(uid id);

            UI void ElementSetText(uid id, const std::string &text);
            UI std::string ElementGetText(uid id);

            UI void ElementSetVisible(uid id, bool state);
            UI bool ElementGetVisible(uid id);

            UI void ElementSetEnable(uid id, bool state);
            UI bool ElementGetEnable(uid id);

            UI void SliderSetValue(uid id, float value);
            UI float SliderGetValue(uid id);

            UI bool CheckBoxGetValue(uid id);
            UI void CheckBoxSetValue(uid id, bool value);

            UI void SliderSetPercentage(uid id, float percentage);
            UI float SliderGetPercentage(uid id);

            UI bool ButtonClicked(uid id);

            UI void PictureBoxSetSprite(uid id, Runtime::SpriteRef sprite);
            UI Runtime::SpriteRef PictureBoxGetSprite(uid id);

                   // Events
            UI void AddEventListener_Click(uid id, UIEventVoid event);

                   // grouping-----------------------------------------------------------------------------------------------------------

            UI bool IsGroup(uid id);
            UI bool GroupShowAsUnique(uid id) throw();
            UI bool GroupShow(uid id) throw();
            UI bool GroupClose(uid id) throw();

                   // other--------------------------------------------------------------------------------------------------------------

            UI bool IsMouseOver();

            UI void SetInteractable(bool state);
            UI bool GetInteractable();

            UI void SetGeneralCallback(UIEventUserData callback, void *userData);

            UI bool PopElement(uid id);
            UI void PopAllElements();

            UI bool FocusedGUI();
        };

#undef UI
    } // namespace UI
} // namespace RoninEngine
