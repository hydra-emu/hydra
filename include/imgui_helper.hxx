#pragma once

#include <IconsMaterialDesign.h>
#include <imgui/imgui.h>
#include <SDL3/SDL.h>
#include <unordered_map>

extern ImFont* big_font;

namespace hydra
{
    struct Url
    {
        static void draw(const char* url)
        {
            static std::unordered_map<const char*, uint32_t> link_colors;
            uint32_t& link_color = link_colors[url];
            ImGui::PushStyleColor(ImGuiCol_Text, link_color);
            ImGui::Text("%s\n", url);
            bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
            bool hover = ImGui::IsItemHovered();
            if (hover)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            }
            if (clicked)
            {
                SDL_OpenURL(url);
            }
            if (link_color != 0xFF8B1A55)
            {
                if (hover)
                {
                    link_color = 0xFFF9C490;
                }
                else
                {
                    link_color = 0xFFFF7B00;
                }
            }
            ImGui::PopStyleColor();
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            min.y = max.y;
            ImGui::GetWindowDrawList()->AddLine(min, max, link_color, 1.0f);
        }
    };

    struct ImGuiHelper
    {
        static float IconWidth()
        {
            static float icon_width = ImGui::CalcTextSize(ICON_MD_10K).x;
            return icon_width;
        }

        static float TextHeight()
        {
            static float text_height = ImGui::CalcTextSize("A").y;
            return text_height;
        }

        static float BigTextHeight()
        {
            static float text_height = 0;
            if (text_height == 0)
            {
                ImGui::PushFont(big_font);
                text_height = ImGui::CalcTextSize("A").y;
                ImGui::PopFont();
            }

            return text_height;
        }
    };
} // namespace hydra