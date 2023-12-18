#include <mainwindow.hxx>
#include <SDL3/SDL_misc.h>

#include <cmrc/cmrc.hpp>
#include <imgui/imgui.h>
#include <imgui_url.hxx>
#include <settings.hxx>

CMRC_DECLARE(hydra);

constexpr int tab_count = 3;

void MainWindow::update()
{
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::BeginGroup();
    float screen_height = ImGui::GetIO().DisplaySize.y;
    float screen_width = ImGui::GetIO().DisplaySize.x;
    float spacing = ImGui::GetStyle().WindowPadding.y * 2 + ImGui::GetStyle().ItemSpacing.y * 2;
    ImVec2 tab_size = ImVec2(screen_width * 0.2f, (screen_height - spacing) / tab_count);
    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
    if (ImGui::Selectable("Home", selected_tab == 0, 0, tab_size))
    {
        selected_tab = 0;
    }
    if (ImGui::Selectable("Settings", selected_tab == 1, 0, tab_size))
    {
        selected_tab = 1;
    }
    if (ImGui::Selectable("About", selected_tab == 2, 0, tab_size))
    {
        selected_tab = 2;
    }
    ImGui::PopStyleVar();
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::BeginChild("##main", ImVec2(0, ImGui::GetWindowHeight() * 0.90f));
    switch (selected_tab)
    {
        case 0:
            draw_cores();
            break;
        case 1:
            draw_settings();
            break;
        case 2:
            draw_about();
            break;
    }
    ImGui::EndChild();
#if defined(HYDRA_WEB) || defined(HYDRA_ANDROID) || defined(HYDRA_IOS)
    static bool dragging = false;
    bool clicked = ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered();
    if (clicked)
    {
        if (!dragging)
        {
            float mouse_delta_x = ImGui::GetIO().MouseDelta.x;
            if (mouse_delta_x > 2.0f)
            {
                selected_tab = (selected_tab - 1 + tab_count) % tab_count;
                dragging = true;
            }
            else if (mouse_delta_x < -2.0f)
            {
                selected_tab = (selected_tab + 1) % tab_count;
                dragging = true;
            }
        }
    }
    else
    {
        dragging = false;
    }
#endif
    ImGui::EndGroup();
}

void MainWindow::draw_cores()
{
    ImGui::Text("Cores");
    ImGui::BeginChild("##cores", ImVec2(0, 0), ImGuiChildFlags_Border);
    auto& cores = Settings::GetCoreInfo();
    for (size_t i = 0; i < cores.size(); i++)
    {
        ImGui::PushID(i);
        ImGui::BeginChild("##core", ImVec2(0, 128 + ImGui::GetStyle().ItemSpacing.y * 4),
                          ImGuiChildFlags_Border);
        ImGui::Image((ImTextureID)(intptr_t)cores[i].icon_texture, ImVec2(128, 128));
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("%s %s (%s)\n", cores[i].core_name.c_str(), cores[i].version.c_str(),
                    cores[i].system_name.c_str());
        ImGui::Separator();
        ImGui::Text("Author: %s\n", cores[i].author.c_str());
        ImGui::Text("License: %s\n", cores[i].license.c_str());
        ImGui::Text("Description: %s\n", cores[i].description.c_str());
        ImGui::Text("Supported files: ");
        ImGui::SameLine(0, 0);
        for (size_t j = 0; j < cores[i].extensions.size(); j++)
        {
            ImGui::Text("*.%s", cores[i].extensions[j].c_str());
            if (j < cores[i].extensions.size() - 1)
            {
                ImGui::SameLine(0, 0);
                ImGui::Text(", ");
                ImGui::SameLine(0, 0);
            }
        }
        ImGui::Text("Website: ");
        ImGui::SameLine(0, 0);
        hydra::Url::draw(cores[i].url.c_str());
        ImGui::EndGroup();

        ImGui::EndChild();
        ImGui::PopID();
    }
    ImGui::EndChild();
}

void MainWindow::draw_about() {}

void MainWindow::draw_settings()
{
    ImGui::Text("Settings");
}