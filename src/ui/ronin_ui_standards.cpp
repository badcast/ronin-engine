#include "ronin.h"
#include "ronin_ui_resources.h"
#include "ronin_render_cache.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <algorithm>
#include <string>
#include <vector>

namespace RoninEngine::Runtime
{
    extern TTF_Font *pDefaultTTFFont;
    extern font2d_t *f2d_default;
}

namespace RoninEngine::UI
{
    using namespace RoninEngine::Runtime;

    void apply_standard_ui_style()
    {
        ImGuiStyle &style = ImGui::GetStyle();
        ImVec4 *colors = style.Colors;

        // Modern slate & obsidian palette
        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.54f, 0.60f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.14f, 0.18f, 0.96f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.11f, 0.15f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.15f, 0.20f, 0.98f);
        colors[ImGuiCol_Border] = ImVec4(0.24f, 0.28f, 0.36f, 0.60f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.21f, 0.27f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.29f, 0.38f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.34f, 0.44f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.13f, 0.16f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.18f, 0.24f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.11f, 0.14f, 0.75f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.16f, 0.20f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.11f, 0.14f, 0.60f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.28f, 0.36f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.32f, 0.38f, 0.48f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.40f, 0.48f, 0.60f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.35f, 0.65f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.65f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.75f, 1.00f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.20f, 0.24f, 0.32f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.36f, 0.50f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.50f, 0.72f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.34f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.36f, 0.48f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.46f, 0.62f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.22f, 0.26f, 0.33f, 0.70f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.35f, 0.60f, 0.90f, 1.00f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.45f, 0.70f, 1.00f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.30f, 0.38f, 0.60f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.35f, 0.60f, 0.90f, 0.80f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.45f, 0.70f, 1.00f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.18f, 0.23f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.36f, 0.48f, 1.00f);
        colors[ImGuiCol_TabSelected] = ImVec4(0.22f, 0.28f, 0.38f, 1.00f);

        // Geometrical adjustments: sleek, rounded, stylish
        style.WindowRounding = 6.0f;
        style.ChildRounding = 4.0f;
        style.FrameRounding = 5.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 5.0f;

        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.TabBorderSize = 0.0f;

        style.WindowPadding = ImVec2(10.0f, 10.0f);
        style.FramePadding = ImVec2(8.0f, 5.0f);
        style.ItemSpacing = ImVec2(8.0f, 6.0f);
        style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    }

    bool init_ui_backend()
    {
        if(ImGui::GetCurrentContext() != nullptr)
            return true;

        if(gscope.renderer == nullptr || gscope.activeWindow == nullptr)
            return false;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = nullptr;
        apply_standard_ui_style();
        ImGui_ImplSDL2_InitForSDLRenderer(gscope.activeWindow, gscope.renderer);
        ImGui_ImplSDLRenderer2_Init(gscope.renderer);
        return true;
    }

    void shutdown_ui_backend()
    {
        if(ImGui::GetCurrentContext() != nullptr)
        {
            ImGui_ImplSDLRenderer2_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();
        }
    }

    bool general_ui_render(UIElement &element, const UIState &uiState, bool &ui_focus)
    {
        bool result = false;
        bool isEnabled = (element.options & ElementEnableMask) && uiState.gui->handle->interactable;

        switch(element.prototype)
        {
            case RGUI_TEXT:
            {
                ImGui::SetCursorScreenPos(ImVec2(static_cast<float>(element.rect.x), static_cast<float>(element.rect.y)));
                if(!isEnabled)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.54f, 0.60f, 1.00f));
                else
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.96f, 0.98f, 1.00f));

                ImGui::TextUnformatted(element.text.c_str());
                ImGui::PopStyleColor();

                if(isEnabled && ImGui::IsItemClicked())
                {
                    result = true;
                }
                break;
            }

            case RGUI_BUTTON:
            {
                ImGui::SetCursorScreenPos(ImVec2(static_cast<float>(element.rect.x), static_cast<float>(element.rect.y)));
                if(!isEnabled)
                    ImGui::BeginDisabled();

                std::string btnLabel = element.text + "##btn_" + std::to_string(element.id);
                ImVec2 btnSize(static_cast<float>(element.rect.w), static_cast<float>(element.rect.h));
                if(ImGui::Button(btnLabel.c_str(), btnSize))
                {
                    result = true;
                    uiState.gui->handle->button_clicked.insert(element.id);
                }

                if(!isEnabled)
                    ImGui::EndDisabled();
                break;
            }

            case RGUI_TEXT_EDIT:
            {
                ImGui::SetCursorScreenPos(ImVec2(static_cast<float>(element.rect.x), static_cast<float>(element.rect.y)));
                ImGui::SetNextItemWidth(static_cast<float>(element.rect.w));

                if(!isEnabled)
                    ImGui::BeginDisabled();

                char buf[512];
                std::strncpy(buf, element.text.c_str(), sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
                std::string editId = "##edit_" + std::to_string(element.id);

                if(ImGui::InputText(editId.c_str(), buf, sizeof(buf)))
                {
                    element.text = buf;
                    result = true;
                }

                if(!isEnabled)
                    ImGui::EndDisabled();
                break;
            }

            case RGUI_HSLIDER:
            {
                ImGui::SetCursorScreenPos(ImVec2(static_cast<float>(element.rect.x), static_cast<float>(element.rect.y)));
                ImGui::SetNextItemWidth(static_cast<float>(element.rect.w));

                if(!isEnabled)
                    ImGui::BeginDisabled();

                std::string sldId = "##slider_" + std::to_string(element.id);
                float val = element.resource.slider.value;
                if(ImGui::SliderFloat(sldId.c_str(), &val, element.resource.slider.min, element.resource.slider.max, "%.2f"))
                {
                    element.resource.slider.value = val;
                    result = true;
                }

                if(!isEnabled)
                    ImGui::EndDisabled();
                break;
            }

            case RGUI_VSLIDER:
            {
                ImGui::SetCursorScreenPos(ImVec2(static_cast<float>(element.rect.x), static_cast<float>(element.rect.y)));

                if(!isEnabled)
                    ImGui::BeginDisabled();

                std::string sldId = "##vslider_" + std::to_string(element.id);
                float val = element.resource.slider.value;
                ImVec2 sldSize(static_cast<float>(element.rect.w), static_cast<float>(element.rect.h));
                if(ImGui::VSliderFloat(sldId.c_str(), sldSize, &val, element.resource.slider.min, element.resource.slider.max, "%.2f"))
                {
                    element.resource.slider.value = val;
                    result = true;
                }

                if(!isEnabled)
                    ImGui::EndDisabled();
                break;
            }

            case RGUI_CHECKBOX:
            {
                ImGui::SetCursorScreenPos(ImVec2(static_cast<float>(element.rect.x), static_cast<float>(element.rect.y)));

                if(!isEnabled)
                    ImGui::BeginDisabled();

                std::string chkId = element.text + "##chk_" + std::to_string(element.id);
                bool val = element.resource.checkbox;
                if(ImGui::Checkbox(chkId.c_str(), &val))
                {
                    element.resource.checkbox = val;
                    result = true;
                }

                if(!isEnabled)
                    ImGui::EndDisabled();
                break;
            }

            case RGUI_DROPDOWN:
            {
                ImGui::SetCursorScreenPos(ImVec2(static_cast<float>(element.rect.x), static_cast<float>(element.rect.y)));
                ImGui::SetNextItemWidth(static_cast<float>(element.rect.w));

                if(!isEnabled)
                    ImGui::BeginDisabled();

                std::string comboId = "##combo_" + std::to_string(element.id);
                if(element.resource.dropdown != nullptr)
                {
                    auto &items = element.resource.dropdown->second;
                    int currentIdx = element.resource.dropdown->first;
                    std::string preview = "";
                    if(currentIdx >= 0 && currentIdx < static_cast<int>(items.size()))
                    {
                        auto it = items.begin();
                        std::advance(it, currentIdx);
                        preview = *it;
                    }

                    if(ImGui::BeginCombo(comboId.c_str(), preview.c_str()))
                    {
                        int idx = 0;
                        for(const auto &item : items)
                        {
                            bool isSelected = (idx == currentIdx);
                            if(ImGui::Selectable(item.c_str(), isSelected))
                            {
                                element.resource.dropdown->first = idx;
                                result = true;
                            }
                            if(isSelected)
                                ImGui::SetItemDefaultFocus();
                            idx++;
                        }
                        ImGui::EndCombo();
                    }
                }

                if(!isEnabled)
                    ImGui::EndDisabled();
                break;
            }

            case RGUI_PICTURE_BOX:
            {
                Sprite *sprite = element.resource.picturebox;
                if(sprite != nullptr && sprite->getImage() != nullptr)
                {
                    SDL_Texture *tex = render_cache_texture(sprite);
                    if(tex != nullptr)
                    {
                        ImGui::SetCursorScreenPos(ImVec2(static_cast<float>(element.rect.x), static_cast<float>(element.rect.y)));
                        float w = element.rect.w == 0 ? static_cast<float>(sprite->width()) : static_cast<float>(element.rect.w);
                        float h = element.rect.h == 0 ? static_cast<float>(sprite->height()) : static_cast<float>(element.rect.h);
                        ImGui::Image(reinterpret_cast<ImTextureID>(tex), ImVec2(w, h));
                        render_cache_unref(tex);
                    }
                }
                break;
            }

            case RGUI_SPRITE_BUTTON:
            {
                Sprite *spr = isEnabled ? (uiState.ms_hover ? element.resource.spriteButton.hover : element.resource.spriteButton.normal) : element.resource.spriteButton.disable;
                if(!spr)
                    spr = element.resource.spriteButton.normal;

                if(spr != nullptr && spr->getImage() != nullptr)
                {
                    SDL_Texture *tex = render_cache_texture(spr);
                    if(tex != nullptr)
                    {
                        ImGui::SetCursorScreenPos(ImVec2(static_cast<float>(element.rect.x), static_cast<float>(element.rect.y)));
                        if(!isEnabled)
                            ImGui::BeginDisabled();

                        std::string btnId = "##sprbtn_" + std::to_string(element.id);
                        float w = element.rect.w == 0 ? static_cast<float>(spr->width()) : static_cast<float>(element.rect.w);
                        float h = element.rect.h == 0 ? static_cast<float>(spr->height()) : static_cast<float>(element.rect.h);
                        if(ImGui::ImageButton(btnId.c_str(), reinterpret_cast<ImTextureID>(tex), ImVec2(w, h)))
                        {
                            result = true;
                            uiState.gui->handle->button_clicked.insert(element.id);
                        }

                        if(!isEnabled)
                            ImGui::EndDisabled();

                        render_cache_unref(tex);
                    }
                }
                break;
            }

            case RGUI_CUSTOM_OVERLAY:
            {
                if(element.resource.overlay.inspector != nullptr)
                {
                    if(!element.resource.overlay.isInit)
                    {
                        element.resource.overlay.inspector->OnInit();
                        element.resource.overlay.isInit = true;
                    }
                    UIData udata {};
                    udata.id = element.id;
                    udata.rect = element.rect;
                    udata.mousePoint = uiState.ms;
                    udata.isMouseDown = Input::GetMouseDown(MouseButton::MouseLeft);
                    udata.isMouseHover = uiState.ms_hover;
                    element.resource.overlay.inspector->OnDraw(&udata);
                }
                break;
            }
        }

        return result;
    }

    void general_ui_event_action(UIElement *element)
    {
        if(element->event == nullptr)
            return;

        switch(element->prototype)
        {
            case RGUI_BUTTON:
            case RGUI_SPRITE_BUTTON:
            case RGUI_TEXT:
            case RGUI_PICTURE_BOX:
            case RGUI_CUSTOM_OVERLAY:
                (reinterpret_cast<UIEventVoid>(element->event))(element->id);
                break;
            case RGUI_DROPDOWN:
                if(element->resource.dropdown != nullptr)
                    (reinterpret_cast<UIEventInteger>(element->event))(element->id, element->resource.dropdown->first);
                break;
            case RGUI_HSLIDER:
            case RGUI_VSLIDER:
                (reinterpret_cast<UIEventFloat>(element->event))(element->id, element->resource.slider.value);
                break;
            case RGUI_CHECKBOX:
                (reinterpret_cast<UIEventBool>(element->event))(element->id, element->resource.checkbox);
                break;
            case RGUI_TEXT_EDIT:
                break;
        }
    }

    void Render_String_ttf(const char *text, int fontSize, const Runtime::Vec2Int &screenPoint, bool alignCenter, bool blend)
    {
        if(text == nullptr || *text == '\0' || pDefaultTTFFont == nullptr || gscope.renderer == nullptr)
            return;

        TTF_SetFontSize(pDefaultTTFFont, fontSize);

        SDL_Surface *surf = blend ? TTF_RenderUTF8_Blended(pDefaultTTFFont, text, RenderUtility::GetColor()) : TTF_RenderUTF8_Solid(pDefaultTTFFont, text, RenderUtility::GetColor());
        if(surf)
        {
            SDL_Texture *texture = SDL_CreateTextureFromSurface(gscope.renderer, surf);
            if(texture)
            {
                SDL_Rect r {screenPoint.x, screenPoint.y, surf->w, surf->h};
                if(alignCenter)
                {
                    r.x -= r.w / 2;
                    r.y -= r.h / 2;
                }
                SDL_RenderCopy(gscope.renderer, texture, nullptr, &r);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surf);
        }
    }

    int Single_TextWidth_WithCyrilic(const std::string &text, int fontSize)
    {
        if(!f2d_default || !f2d_default->data)
            return 0;

        int width = 0;
        for(auto iter = begin(text); iter != end(text); ++iter)
        {
            width += f2d_default->data[(unsigned char) *iter].w;
        }
        return width;
    }
} // namespace RoninEngine::UI
