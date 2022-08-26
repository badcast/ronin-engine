#pragma once
#include "dependency.h"

using namespace RoninEngine;

namespace RoninEngine::UI {

#define CI

enum { NOPARENT = 0 };

///Тип идентификатора GUI
typedef std::uint8_t uid;

enum TextRandomizer_Format { All = -1, OnlyText = 0, OnlyNumber = 1, OnlyReal = 3, MaskText = 2, MaskNumber = 4, MaskReal = 8, MaskUpperChar = 16, MaskLowwerChar = 32 };

struct {
    Vec2Int dropdownSize = Vec2Int(120, 30);
    Vec2Int buttonSize = Vec2Int(120, 30);
} defaultMakets;

class GUI final {
   private:
    struct {
        // controls
        std::vector<UIElement> elements;
        std::list<uid> layers;
    } ui_layer;

    CI ui_callback callback;
    CI void *callbackData;
    CI Level *m_level;
    CI bool hitCast;
    CI bool _focusedUI;

    /// Register ui element and get unique ID
    CI std::list<uid> get_groups();
    CI friend uid call_register_ui(GUI *gui, uid parent) throw();
    CI friend UIElement &call_get_element(GUI *gui, uid id);
    CI UIElement &getElement(uid id);
    CI bool has_action(void *outPos);

   public:
    bool visible;

    CI GUI(Level *);
    CI virtual ~GUI();

    CI bool Has_ID(uid id);

    CI uid Create_Group(const RoninEngine::Runtime::Rect &rect);
    CI uid Create_Group();

    CI uid Push_Label(const std::string &text, const RoninEngine::Runtime::Rect &rect, const int &fontWidth = 13, uid parent = NOPARENT);
    CI uid Push_Label(const std::string &text, const Vec2Int &point, const int &fontWidth = 13, uid parent = NOPARENT);
    CI uid Push_Button(const std::string &text, const RoninEngine::Runtime::Rect &rect, uid parent = NOPARENT);
    CI uid Push_Button(const std::string &text, const Vec2Int point, uid parent = NOPARENT);
    CI uid Push_DisplayRandomizer(TextRandomizer_Format format, const Vec2Int &point, uid parent = NOPARENT);
    CI uid Push_DisplayRandomizer(TextRandomizer_Format format = TextRandomizer_Format::All, uid parent = NOPARENT);
    CI uid Push_DisplayRandomizer_Text(const std::string &text, const Vec2Int &point, uid parent = NOPARENT);
    CI uid Push_DisplayRandomizer_Number(const int &min, const int &max, TextAlign textAlign, uid parent = NOPARENT);
    CI uid Push_TextureStick(Texture *texture, const RoninEngine::Runtime::Rect &rect, uid parent = NOPARENT);
    CI uid Push_TextureStick(Texture *texture, const Vec2Int point, uid parent = NOPARENT);
    CI uid Push_TextureAnimator(Timeline *timeline, const RoninEngine::Runtime::Rect &rect, uid parent = NOPARENT);
    CI uid Push_TextureAnimator(Timeline *timeline, const Vec2Int &point, uid parent = NOPARENT);
    CI uid Push_TextureAnimator(const std::list<Texture *> &roads, float duration, TimelineOptions option, const RoninEngine::Runtime::Rect &rect,
                                uid parent = NOPARENT);  // 930923840293840
    CI uid Push_TextureAnimator(const std::list<Texture *> &roads, float duration, TimelineOptions option, const Vec2Int &point, uid parent = NOPARENT);
    template <typename Container>
    CI uid Push_DropDown(const Container &elements, const Runtime::Vec2Int &point, uid parent = NOPARENT) {
        return Push_DropDown(elements, 0, point, parent);
    }
    template <typename Container>
    CI uid Push_DropDown(const Container &elements, int index, const Runtime::Vec2Int &point, uid parent = NOPARENT) {
        return Push_DropDown(elements, index, Runtime::Rect(point.x, point.y, defaultMakets.dropdownSize.x, defaultMakets.dropdownSize.y), parent);
    }
    template <typename Container>
    CI uid Push_DropDown(const Container &elements, int index, const Runtime::Rect &rect, uid parent = NOPARENT);

    // property-----------------------------------------------------------------------------------------------------------

    CI void *Resources(uid id);
    CI void Resources(uid id, void *data);

    CI RoninEngine::Runtime::Rect Rect(uid id);
    CI void Rect(uid id, const RoninEngine::Runtime::Rect &rect);

    CI std::string Text(uid id);
    CI void Text(uid id, const std::string &text);

    CI void Visible(uid id, bool state);
    CI bool Visible(uid id);

    CI void Enable(uid id, bool state);
    CI bool Enable(uid id);

    // grouping-----------------------------------------------------------------------------------------------------------

    CI bool Is_Group(uid id);
    CI void Show_GroupUnique(uid id) throw();
    CI void Show_Group(uid id) throw();
    CI bool Close_Group(uid id) throw();

    // other--------------------------------------------------------------------------------------------------------------

    CI void Cast(bool state);
    CI bool Cast();

    CI void Register_Callback(ui_callback callback, void *userData);

    CI bool Pop_Element(uid id);
    CI void RemoveAll();

    CI void Do_Present(SDL_Renderer *renderer);

    CI void GUI_SetMainColorRGB(uint32_t RGB);
    CI void GUI_SetMainColorRGBA(uint32_t ARGB);

    CI bool Focused_UI();
};

extern GUI *guiInstance;

#undef CI
}  // namespace RoninEngine::UI
