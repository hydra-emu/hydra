#pragma once

#include <IconsMaterialDesign.h>
#include <imgui/imgui.h>

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
    };
} // namespace hydra