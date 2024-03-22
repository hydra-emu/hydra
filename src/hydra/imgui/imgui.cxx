#include <hydra/imgui/imgui.hxx>
#include <imgui.h>

namespace hydra::imgui
{
    void init()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        ImGui::StyleColorsDark();
    }

    void shutdown()
    {
        ImGui::DestroyContext();
    }
} // namespace hydra::imgui