#pragma once

#include <IconsMaterialDesign.h>
#include <imgui/imgui.h>

extern ImFont* big_font;

namespace hydra
{
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