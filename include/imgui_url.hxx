#pragma once

#include <cstdint>
#include <imgui/imgui.h>
#include <SDL3/SDL_misc.h>
#include <unordered_map>

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
} // namespace hydra
