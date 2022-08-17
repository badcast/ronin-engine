#pragma once
#include "dependency.h"

using namespace RoninEngine;

namespace RoninEngine::UI {

#define CI

#define NOPARENT 0

///Тип идентификатора GUI
typedef std::uint8_t uid;

enum TextRandomizer_Format {
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

class GUI final {
   private:
    struct {
        // controls
        std::vector<RenderData> controls;
        std::list<uid> _rendering;
    } m_Sticks;

    CI ui_callback callback;
    CI void *callbackData;
    Level *m_level;
    bool hitCast;
    bool _focusedUI;

    ///Регистрирует хранилище для пользовательских инструментов и возвращает id
    CI std::list<uid> get_groups();
    CI uid call_register_ui(const uid &parent = NOPARENT) throw();
    CI RenderData &ID(const uid &id);
    CI bool has_action(void *outPos);

   public:
    bool visible;

    CI GUI(Level *m_scene);
    CI virtual ~GUI();

    CI bool Has_ID(const uid &id);

    CI uid Create_Group(const RoninEngine::Runtime::Rect &rect);
    CI uid Create_Group();

    CI uid Push_Label(const std::string &text, const RoninEngine::Runtime::Rect &rect, const int &fontWidth = 13, const uid &parent = NOPARENT);
    CI uid Push_Label(const std::string &text, const Vec2Int &point, const int &fontWidth = 13, const uid &parent = NOPARENT);
    CI uid Push_Button(const std::string &text, const RoninEngine::Runtime::Rect &rect, const uid &parent = NOPARENT);
    CI uid Push_Button(const std::string &text, const Vec2Int point, const uid &parent = NOPARENT);
    CI uid Push_DisplayRandomizer(TextRandomizer_Format format, const Vec2Int &point, const uid &parent = NOPARENT);
    CI uid Push_DisplayRandomizer(TextRandomizer_Format format = TextRandomizer_Format::All, const uid &parent = NOPARENT);
    CI uid Push_DisplayRanomizer_Text(const std::string &text, const Vec2Int &point, const uid &parent = 0);
    CI uid Push_DisplayRanomizer_Number(const int &min, const int &max, TextAlign textAlign, const uid &parent = NOPARENT);
    CI uid Push_TextureStick(Texture *texture, const RoninEngine::Runtime::Rect &rect, const uid &parent = NOPARENT);
    CI uid Push_TextureStick(Texture *texture, const Vec2Int point, const uid &parent = NOPARENT);
    CI uid Push_TextureAnimator(Timeline *timeline, const RoninEngine::Runtime::Rect &rect, const uid &parent = NOPARENT);
    CI uid Push_TextureAnimator(Timeline *timeline, const Vec2Int &point, const uid &parent = NOPARENT);
    CI uid Push_TextureAnimator(const std::list<Texture *> &roads, float duration, TimelineOptions option, const RoninEngine::Runtime::Rect &rect,
                                const uid &parent = NOPARENT);
    CI uid Push_TextureAnimator(const std::list<Texture *> &roads, float duration, TimelineOptions option, const Vec2Int &point,
                                const uid &parent = NOPARENT);

    // property-----------------------------------------------------------------------------------------------------------

    CI void *Resources(const uid &id);
    CI void Resources(const uid &id, void *data);

    CI RoninEngine::Runtime::Rect Rect(const uid &id);
    CI void Rect(const uid &id, const RoninEngine::Runtime::Rect &rect);

    CI std::string Text(const uid &id);
    CI void Text(const uid &id, const std::string &text);

    CI void Visible(const uid &id, bool state);
    CI bool Visible(const uid &id);

    CI void Enable(const uid &id, bool state);
    CI bool Enable(const uid &id);

    // grouping-----------------------------------------------------------------------------------------------------------

    CI bool Is_Group(const uid &id);
    CI void Show_GroupUnique(const uid &id) throw();
    CI void Show_Group(const uid &id) throw();
    CI bool Close_Group(const uid &id) throw();

    // other--------------------------------------------------------------------------------------------------------------

    CI void Cast(bool state);
    CI bool Cast();

    CI void Register_Callback(ui_callback callback, void *userData);

    CI bool Remove(const uid &id);
    CI void RemoveAll();

    CI void Do_Present(SDL_Renderer *renderer);

    CI void GUI_SetMainColorRGB(uint32_t RGB);
    CI void GUI_SetMainColorRGBA(uint32_t ARGB);

    CI bool Focused_UI();
};

extern GUI *guiInstance;

#undef CI
}  // namespace RoninEngine::UI
