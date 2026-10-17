#include "editor_icons.h"
#include <cmath>
#include <algorithm>

namespace RoninEditor
{
    void EditorIcons::DrawIcon(ImDrawList *drawList, const ImVec2 &pMin, const ImVec2 &pMax, IconType icon, ImU32 color)
    {
        if (!drawList) return;

        float w = pMax.x - pMin.x;
        float h = pMax.y - pMin.y;
        float cx = pMin.x + w * 0.5f;
        float cy = pMin.y + h * 0.5f;
        float pad = std::min(w, h) * 0.18f;
        float x0 = pMin.x + pad;
        float y0 = pMin.y + pad;
        float x1 = pMax.x - pad;
        float y1 = pMax.y - pad;
        float rw = x1 - x0;
        float rh = y1 - y0;

        switch (icon)
        {
            case IconType::Play:
            {
                // Right-pointing filled triangle
                drawList->AddTriangleFilled(
                    ImVec2(x0 + rw * 0.15f, y0),
                    ImVec2(x1, cy),
                    ImVec2(x0 + rw * 0.15f, y1),
                    color);
                break;
            }
            case IconType::Pause:
            {
                // Two vertical rounded bars
                float barW = rw * 0.28f;
                drawList->AddRectFilled(ImVec2(x0, y0), ImVec2(x0 + barW, y1), color, 1.5f);
                drawList->AddRectFilled(ImVec2(x1 - barW, y0), ImVec2(x1, y1), color, 1.5f);
                break;
            }
            case IconType::Stop:
            {
                // Filled square
                drawList->AddRectFilled(ImVec2(x0 + rw * 0.12f, y0 + rh * 0.12f), ImVec2(x1 - rw * 0.12f, y1 - rh * 0.12f), color, 2.0f);
                break;
            }
            case IconType::Step:
            {
                // Triangle + vertical bar
                drawList->AddTriangleFilled(
                    ImVec2(x0, y0),
                    ImVec2(x0 + rw * 0.65f, cy),
                    ImVec2(x0, y1),
                    color);
                drawList->AddRectFilled(ImVec2(x1 - rw * 0.20f, y0), ImVec2(x1, y1), color, 1.0f);
                break;
            }
            case IconType::Move:
            {
                // 4-way translation cross with arrows
                float th = 1.5f;
                drawList->AddLine(ImVec2(x0 + 2.0f, cy), ImVec2(x1 - 2.0f, cy), color, th);
                drawList->AddLine(ImVec2(cx, y0 + 2.0f), ImVec2(cx, y1 - 2.0f), color, th);
                // Arrowheads
                drawList->AddTriangleFilled(ImVec2(x1, cy), ImVec2(x1 - 3.5f, cy - 2.5f), ImVec2(x1 - 3.5f, cy + 2.5f), color);
                drawList->AddTriangleFilled(ImVec2(x0, cy), ImVec2(x0 + 3.5f, cy - 2.5f), ImVec2(x0 + 3.5f, cy + 2.5f), color);
                drawList->AddTriangleFilled(ImVec2(cx, y0), ImVec2(cx - 2.5f, y0 + 3.5f), ImVec2(cx + 2.5f, y0 + 3.5f), color);
                drawList->AddTriangleFilled(ImVec2(cx, y1), ImVec2(cx - 2.5f, y1 - 3.5f), ImVec2(cx + 2.5f, y1 - 3.5f), color);
                break;
            }
            case IconType::Rotate:
            {
                // Circular arc with arrow
                float r = std::min(rw, rh) * 0.44f;
                drawList->PathArcTo(ImVec2(cx, cy), r, 0.3f, 4.8f, 16);
                drawList->PathStroke(color, false, 1.8f);
                // Arrowhead at top
                float ax = cx + r * std::cos(0.3f);
                float ay = cy + r * std::sin(0.3f);
                drawList->AddTriangleFilled(ImVec2(ax, ay), ImVec2(ax - 2.0f, ay - 4.0f), ImVec2(ax + 3.5f, ay - 2.0f), color);
                break;
            }
            case IconType::Scale:
            {
                // Inner square and outer square corner
                drawList->AddRect(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), color, 0.0f, 0, 1.5f);
                drawList->AddRectFilled(ImVec2(x1 - 4.5f, y0), ImVec2(x1, y0 + 4.5f), color);
                drawList->AddRectFilled(ImVec2(x0, y1 - 4.5f), ImVec2(x0 + 4.5f, y1), color);
                drawList->AddRectFilled(ImVec2(x1 - 4.5f, y1 - 4.5f), ImVec2(x1, y1), color);
                break;
            }
            case IconType::Terrain:
            {
                // Mountain peaks silhouette
                ImVec2 pts[5] = {
                    ImVec2(x0, y1),
                    ImVec2(x0 + rw * 0.35f, y0 + rh * 0.15f),
                    ImVec2(x0 + rw * 0.60f, y0 + rh * 0.50f),
                    ImVec2(x0 + rw * 0.80f, y0 + rh * 0.30f),
                    ImVec2(x1, y1)
                };
                drawList->AddPolyline(pts, 5, color, false, 1.6f);
                drawList->AddLine(ImVec2(x0, y1), ImVec2(x1, y1), color, 1.6f);
                break;
            }
            case IconType::Reset:
            {
                // Counter-clockwise reload arrow
                float r = std::min(rw, rh) * 0.42f;
                drawList->PathArcTo(ImVec2(cx, cy), r, 0.6f, 5.2f, 16);
                drawList->PathStroke(color, false, 1.6f);
                float ax = cx + r * std::cos(0.6f);
                float ay = cy + r * std::sin(0.6f);
                drawList->AddTriangleFilled(ImVec2(ax, ay), ImVec2(ax + 3.5f, ay + 3.0f), ImVec2(ax + 4.0f, ay - 2.0f), color);
                break;
            }
            case IconType::Folder:
            {
                // Classic tabbed folder silhouette
                float tabW = rw * 0.45f;
                float tabH = rh * 0.25f;
                drawList->AddRectFilled(ImVec2(x0, y0), ImVec2(x0 + tabW, y0 + tabH + 2.0f), color, 1.5f);
                drawList->AddRectFilled(ImVec2(x0, y0 + tabH), ImVec2(x1, y1), color, 2.0f);
                break;
            }
            case IconType::File:
            {
                // Document silhouette with folded corner
                float fold = rw * 0.32f;
                ImVec2 pts[5] = {
                    ImVec2(x0, y0),
                    ImVec2(x1 - fold, y0),
                    ImVec2(x1, y0 + fold),
                    ImVec2(x1, y1),
                    ImVec2(x0, y1)
                };
                drawList->AddConvexPolyFilled(pts, 5, color);
                // Fold triangle
                drawList->AddTriangle(ImVec2(x1 - fold, y0), ImVec2(x1 - fold, y0 + fold), ImVec2(x1, y0 + fold), IM_COL32(20, 20, 24, 200), 1.0f);
                break;
            }
            case IconType::World:
            {
                // Globe with meridian/parallel arcs
                float r = std::min(rw, rh) * 0.46f;
                drawList->AddCircle(ImVec2(cx, cy), r, color, 20, 1.5f);
                drawList->AddLine(ImVec2(cx - r, cy), ImVec2(cx + r, cy), color, 1.2f);
                drawList->AddLine(ImVec2(cx, cy - r), ImVec2(cx, cy + r), color, 1.2f);
                // Elliptical meridian arc
                drawList->AddEllipse(ImVec2(cx, cy), ImVec2(r * 0.45f, r), color, 0.0f, 16, 1.2f);
                break;
            }
            case IconType::Sprite:
            {
                // Framed picture with mountain & sun
                drawList->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), color, 2.0f, 0, 1.5f);
                drawList->AddCircleFilled(ImVec2(x0 + rw * 0.30f, y0 + rh * 0.35f), rw * 0.12f, color);
                ImVec2 mPts[3] = {
                    ImVec2(x0 + 2.0f, y1 - 2.0f),
                    ImVec2(x0 + rw * 0.55f, y0 + rh * 0.45f),
                    ImVec2(x1 - 2.0f, y1 - 2.0f)
                };
                drawList->AddPolyline(mPts, 3, color, false, 1.4f);
                break;
            }
            case IconType::Audio:
            {
                // Speaker horn + sound wave
                float hw = rw * 0.45f;
                drawList->AddRectFilled(ImVec2(x0, cy - rh * 0.22f), ImVec2(x0 + hw * 0.45f, cy + rh * 0.22f), color);
                ImVec2 horn[3] = {
                    ImVec2(x0 + hw * 0.45f, cy - rh * 0.22f),
                    ImVec2(x0 + hw, y0),
                    ImVec2(x0 + hw, y1)
                };
                drawList->AddConvexPolyFilled(horn, 3, color);
                // Sound waves
                drawList->PathArcTo(ImVec2(x0 + hw * 0.5f, cy), rw * 0.38f, -0.7f, 0.7f, 8);
                drawList->PathStroke(color, false, 1.4f);
                drawList->PathArcTo(ImVec2(x0 + hw * 0.5f, cy), rw * 0.55f, -0.6f, 0.6f, 8);
                drawList->PathStroke(color, false, 1.4f);
                break;
            }
            case IconType::Camera:
            {
                // Camera box with lens and top flash
                float topW = rw * 0.35f;
                drawList->AddRectFilled(ImVec2(cx - topW * 0.5f, y0), ImVec2(cx + topW * 0.5f, y0 + rh * 0.22f), color, 1.5f);
                drawList->AddRectFilled(ImVec2(x0, y0 + rh * 0.18f), ImVec2(x1, y1), color, 2.5f);
                drawList->AddCircle(ImVec2(cx, y0 + rh * 0.58f), rw * 0.22f, IM_COL32(24, 24, 28, 255), 16, 2.0f);
                break;
            }
            case IconType::Light:
            {
                // Radiant sun lightbulb
                float r = std::min(rw, rh) * 0.28f;
                drawList->AddCircleFilled(ImVec2(cx, cy), r, color, 16);
                for (int a = 0; a < 8; ++a)
                {
                    float angle = a * (3.14159265f / 4.0f);
                    float xA = cx + (r + 1.5f) * std::cos(angle);
                    float yA = cy + (r + 1.5f) * std::sin(angle);
                    float xB = cx + (r + 4.5f) * std::cos(angle);
                    float yB = cy + (r + 4.5f) * std::sin(angle);
                    drawList->AddLine(ImVec2(xA, yA), ImVec2(xB, yB), color, 1.4f);
                }
                break;
            }
            case IconType::Search:
            {
                // Magnifying glass
                float r = std::min(rw, rh) * 0.32f;
                float gx = cx - 2.0f;
                float gy = cy - 2.0f;
                drawList->AddCircle(ImVec2(gx, gy), r, color, 16, 1.8f);
                drawList->AddLine(ImVec2(gx + r * 0.7f, gy + r * 0.7f), ImVec2(x1, y1), color, 2.4f);
                break;
            }
            case IconType::Delete:
            {
                // Trashcan with lid
                drawList->AddRectFilled(ImVec2(x0 + 1.0f, y0), ImVec2(x1 - 1.0f, y0 + 2.5f), color, 1.0f);
                drawList->AddRect(ImVec2(x0 + 3.0f, y0 + 3.5f), ImVec2(x1 - 3.0f, y1), color, 1.5f, 0, 1.4f);
                drawList->AddLine(ImVec2(cx - 2.5f, y0 + 6.0f), ImVec2(cx - 2.5f, y1 - 3.0f), color, 1.2f);
                drawList->AddLine(ImVec2(cx + 2.5f, y0 + 6.0f), ImVec2(cx + 2.5f, y1 - 3.0f), color, 1.2f);
                break;
            }
            case IconType::Plus:
            {
                // Geometric cross
                float th = 2.0f;
                drawList->AddLine(ImVec2(x0 + 2.0f, cy), ImVec2(x1 - 2.0f, cy), color, th);
                drawList->AddLine(ImVec2(cx, y0 + 2.0f), ImVec2(cx, y1 - 2.0f), color, th);
                break;
            }
        }
    }

    bool EditorIcons::IconButton(const char *id, IconType icon, const ImVec2 &size, bool active, ImU32 iconColor)
    {
        ImGui::PushID(id);
        ImVec2 p = ImGui::GetCursorScreenPos();
        bool pressed = ImGui::Button("##IconBtn", size);
        bool hovered = ImGui::IsItemHovered();

        ImU32 col = active ? EditorPalette::TextWhite : (hovered ? EditorPalette::TextWhite : iconColor);
        if (active)
        {
            ImDrawList *drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), EditorPalette::Primary, 3.0f);
        }

        DrawIcon(ImGui::GetWindowDrawList(), p, ImVec2(p.x + size.x, p.y + size.y), icon, col);
        ImGui::PopID();
        return pressed;
    }

    void EditorIcons::IconLabel(IconType icon, const char *label, ImU32 iconColor, float iconSize)
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        float lineH = ImGui::GetTextLineHeight();
        float yOffset = (lineH - iconSize) * 0.5f;
        ImVec2 iconMin(p.x, p.y + yOffset);
        ImVec2 iconMax(p.x + iconSize, p.y + yOffset + iconSize);

        DrawIcon(ImGui::GetWindowDrawList(), iconMin, iconMax, icon, iconColor);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + iconSize + 6.0f);
        ImGui::TextUnformatted(label);
    }
} // namespace RoninEditor
