#include <mainwindow.hxx>
#include <SDL3/SDL_misc.h>

#include <cmrc/cmrc.hpp>
#include <imgui/imgui.h>
#include <imgui_url.hxx>
#include <numbers>
#include <settings.hxx>

CMRC_DECLARE(hydra);

constexpr int tab_count = 3;
constexpr const char* names[tab_count] = {"Cores", "Settings", "About"};

MainWindow::MainWindow()
{
    selected_y = ImGui::GetStyle().WindowPadding.y + ImGui::GetStyle().ItemSpacing.y;
    core_directory.resize(4096);
    std::string core_dir = Settings::Get("core_path");
    strncpy(core_directory.data(), core_dir.c_str(), core_directory.size());

    if (core_dir.empty())
    {
        hydra::log("Core path is empty, shouldn't happen");
    }

    for (size_t i = 0; i < stars.size(); i++)
    {
        float angle = (rand() % 100) / 100.0f * 2.0f * std::numbers::pi;
        float dist = (rand() % 100) / 100.0f * 50.0f;
        stars[i].x = 50.0f + dist * cos(angle);
        stars[i].y = 50.0f + dist * sin(angle);
        stars[i].vel_x = ((rand() % 100) / 200.0f) - 0.25f;
        stars[i].vel_y = ((rand() % 100) / 200.0f) - 0.25f;
    }

    if (Settings::Get("fancy_gui").empty())
    {
        Settings::Set("fancy_gui", "true");
    }
    fancy_gui = Settings::Get("fancy_gui") == "true";
}

void MainWindow::update()
{
    ImGui::GetStyle().WindowRounding = 0.0f;
    float screen_height = ImGui::GetIO().DisplaySize.y;
    float screen_width = ImGui::GetIO().DisplaySize.x;
    float spacing = ImGui::GetStyle().WindowPadding.y * 2 + ImGui::GetStyle().ItemSpacing.y * 2;
    ImVec2 tab_size = ImVec2(screen_width * 0.2f, (screen_height - spacing) / tab_count);
    float target_y = selected_tab * tab_size.y + ImGui::GetStyle().WindowPadding.y +
                     ImGui::GetStyle().ItemSpacing.y;
    selected_y = selected_y * 0.9f + target_y * 0.1f;
    ImVec2 min(ImGui::GetCursorPosX(), selected_y);
    ImVec2 max(tab_size.x + min.x, tab_size.y + min.y);
    ImVec2 center = ImVec2(min.x + tab_size.x / 2, min.y + tab_size.y / 2);
    float radius = std::min(tab_size.x, tab_size.y) * 0.5f;
    draw_stars(center, radius);
    ImGui::BeginGroup();
    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddCircle(center, radius, 0xFFFFFFFF, 0, 2.0f);

    // We draw our own animated rectangle so disable these
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
    for (int i = 0; i < tab_count; i++)
    {
        min.y =
            i * tab_size.y + ImGui::GetStyle().WindowPadding.y + ImGui::GetStyle().ItemSpacing.y;
        max.y = tab_size.y + min.y;
        bool hovered = ImGui::IsMouseHoveringRect(min, max);
        if (hovered)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 1, 0.75));
        if (ImGui::Selectable(names[i], selected_tab == i, ImGuiSelectableFlags_SpanAllColumns,
                              tab_size))
        {
            selected_tab = i;
        }
        if (hovered)
            ImGui::PopStyleColor();
    }
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    ImGui::EndGroup();
    ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x * 2);
    ImGui::BeginGroup();
    float cursor_x = ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x;
    draw_list->AddLine(ImVec2(cursor_x, ImGui::GetStyle().WindowPadding.y),
                       ImVec2(cursor_x, screen_height - ImGui::GetStyle().WindowPadding.y),
                       0x80FFFFFF, 0.5f);
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
    auto& cores = Settings::GetCoreInfo();
    // Calculate the height of the above text and the input text below
    float widget_height = ImGui::CalcTextSize("").y * 2 + ImGui::GetStyle().FramePadding.y * 2.0f +
                          ImGui::GetStyle().ItemSpacing.y * 2.0f;
    ImGui::BeginChild("##cores", ImVec2(0, ImGui::GetWindowHeight() - widget_height),
                      ImGuiChildFlags_Border);
    float image_width = ImGui::GetWindowWidth() * 0.1f;
    image_width = std::min(image_width, 128.0f);
    image_width = std::max(image_width, 64.0f);
    for (size_t i = 0; i < cores.size(); i++)
    {
        ImGui::PushID(i);
        ImGui::BeginChild("##core", ImVec2(0, 128 + ImGui::GetStyle().ItemSpacing.y * 4),
                          ImGuiChildFlags_Border);
        ImGui::Image((ImTextureID)(intptr_t)cores[i].icon_texture,
                     ImVec2(image_width, image_width));
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

    float button_width = ImGui::GetWindowWidth() * 0.2f;
    button_width = std::min(button_width, 128.0f);
    button_width = std::max(button_width, 64.0f);
    ImGui::PushItemWidth(ImGui::GetWindowWidth() - button_width - ImGui::GetStyle().ItemSpacing.x);
    ImGui::InputText("##coredir", core_directory.data(), core_directory.size(),
                     ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button("Browse", ImVec2(button_width, 0)))
    {
    }
}

void MainWindow::draw_about() {}

void MainWindow::draw_settings()
{
    ImGui::Text("Settings");
    if (ImGui::Checkbox("Enable fancy GUI", &fancy_gui))
    {
        Settings::Set("fancy_gui", fancy_gui ? "true" : "false");
    }
    ImGui::SetItemTooltip("Enable fancy GUI features that may hinder performance");
}

void MainWindow::draw_stars(ImVec2 center, float radius)
{
    if (!fancy_gui)
        return;

    ImVec2 min(center.x - radius, center.y - radius);
    ImVec2 max(center.x + radius, center.y + radius);
    ImVec2 size(max.x - min.x, max.y - min.y);
    for (size_t i = 0; i < stars.size(); i++)
    {
        float x = stars[i].x - 50.0f;
        float y = stars[i].y - 50.0f;
        float dist = sqrt(x * x + y * y);
        if (dist > 50.0f)
        {
            stars[i].x = 50.0f;
            stars[i].y = 50.0f;
        }

        float dx = (stars[i].x + stars[i].vel_x) - 50.0f;
        float dy = (stars[i].y + stars[i].vel_y) - 50.0f;
        float dist2 = sqrt(dx * dx + dy * dy);
        if (dist2 > 50.0f)
        {
            float dot = dx * stars[i].vel_x + dy * stars[i].vel_y;
            stars[i].vel_x -= 2.0f * dot * dx / (dist2 * dist2);
            stars[i].vel_y -= 2.0f * dot * dy / (dist2 * dist2);
        }

        stars[i].x += stars[i].vel_x;
        stars[i].y += stars[i].vel_y;

        ImVec2 pos(min.x + size.x * stars[i].x / 100.0f, min.y + size.y * stars[i].y / 100.0f);
        ImGui::GetWindowDrawList()->AddCircleFilled(pos, 0.2f, 0xCCCCCCCC);

        for (size_t j = i + 1; j < stars.size(); j++)
        {
            float dist_x = stars[i].x - stars[j].x;
            float dist_y = stars[i].y - stars[j].y;
            float dist = sqrt(dist_x * dist_x + dist_y * dist_y);
            float max_dist = 50.0f;
            if (dist < max_dist)
            {
                uint32_t color = (uint32_t)(180.0f * (1.0f - dist / max_dist));
                color = color << 24 | color << 16 | color << 8 | color;
                ImVec2 pos2(min.x + size.x * stars[j].x / 100.0f,
                            min.y + size.y * stars[j].y / 100.0f);
                ImGui::GetWindowDrawList()->AddLine(pos, pos2, color);
            }
        }

        // Connect to cursor if close enough
        // first check if cursor is in circle
        float dist_x = stars[i].x - (ImGui::GetIO().MousePos.x - min.x) / size.x * 100.0f;
        float dist_y = stars[i].y - (ImGui::GetIO().MousePos.y - min.y) / size.y * 100.0f;
        dist = sqrt(dist_x * dist_x + dist_y * dist_y);

        float max_dist = 40.0f;
        if (dist < max_dist)
        {
            uint32_t color = 180.0f * (1.0f - dist / max_dist);
            color = color << 24 | color << 16 | color << 8 | color;
            ImVec2 pos2(ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
            ImGui::GetWindowDrawList()->AddLine(pos, pos2, color);
        }
    }
}