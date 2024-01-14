#include <gamewindow.hxx>
#include <mainwindow.hxx>
#include <SDL3/SDL_misc.h>

#include <cmrc/cmrc.hpp>
#include <IconsMaterialDesign.h>
#include <imgui/imgui.h>
#include <imgui_helper.hxx>
#include <memory>
#include <numbers>
#include <settings.hxx>
#include <strings.hxx>

CMRC_DECLARE(hydra);

extern ImFont* small_font;
extern ImFont* big_font;

constexpr size_t tab_count = 4;
constexpr size_t games_tab = 0;
constexpr size_t cores_tab = 1;
constexpr size_t settings_tab = 2;
constexpr size_t about_tab = 3;

constexpr size_t recent_max = 50;
constexpr const char* names[tab_count] = {"Games", "Cores", "Settings", "About"};
constexpr const char* icons[tab_count] = {ICON_MD_GAMES, ICON_MD_MEMORY, ICON_MD_SETTINGS,
                                          ICON_MD_INFO};

MainWindow::MainWindow()
    : rom_picker("rom_picker", "Load ROM", [this](const char* path) { loadRom(path); })
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

    for (size_t i = 0; i < recent_max; i++)
    {
        std::string recent = Settings::Get("recent_" + std::to_string(i));
        if (!recent.empty())
        {
            recent_roms.push_back(recent);
        }
        else
        {
            break;
        }
    }
}

void MainWindow::update_impl()
{
    ImGui::GetStyle().WindowRounding = 0.0f;
    float screen_height = ImGui::GetIO().DisplaySize.y - ImGui::GetStyle().WindowPadding.y * 2;
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

    // We draw our own animated rectangle so disable these
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
    ImGui::PushFont(big_font);
    if (biggest_tab_size == 0)
    {
        biggest_tab_size = ImGui::CalcTextSize("Settings").x;
    }
    bool use_icons = biggest_tab_size > tab_size.x;
    for (int i = 0; i < tab_count; i++)
    {
        min.y =
            i * tab_size.y + ImGui::GetStyle().WindowPadding.y + ImGui::GetStyle().ItemSpacing.y;
        max.y = tab_size.y + min.y;
        bool hovered = ImGui::IsMouseHoveringRect(min, max) && ImGui::IsWindowHovered();
#if defined(HYDRA_WEB) || defined(HYDRA_ANDROID) || defined(HYDRA_IOS)
        hovered &= ImGui::IsMouseDown(ImGuiMouseButton_Left);
#endif
        if (hovered)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 1, 0.75));
        if (ImGui::Selectable(use_icons ? icons[i] : names[i], selected_tab == i,
                              ImGuiSelectableFlags_SpanAllColumns, tab_size))
        {
            selected_tab = i;
        }
        if (hovered)
            ImGui::PopStyleColor();
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    ImGui::EndGroup();
    ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x * 2);
    ImGui::BeginGroup();
    float cursor_x = ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddLine(
        ImVec2(cursor_x, ImGui::GetStyle().WindowPadding.y),
        ImVec2(cursor_x, ImGui::GetIO().DisplaySize.y - ImGui::GetStyle().WindowPadding.y),
        0x80FFFFFF, 0.5f);
    ImGui::BeginGroup();
    switch (selected_tab)
    {
        case games_tab:
            draw_games();
            break;
        case cores_tab:
            draw_cores();
            break;
        case settings_tab:
            draw_settings();
            break;
        case about_tab:
            draw_about();
            break;
    }
    ImGui::EndGroup();
    ImGui::EndGroup();
}

void MainWindow::update()
{
    if (!game_window || !game_window->isFullscreen())
        update_impl();
    if (game_window)
    {
        UpdateResult result = game_window->update();
        if (result == UpdateResult::Quit)
        {
            game_window.reset();
        }
    }
}

void MainWindow::loadRom(const std::filesystem::path& path)
{
    if (pending_rom_load)
    {
        hydra::log("Pending ROM load while loadRom is called, shouldn't happen");
        return;
    }

    // Find a core for this extension
    std::string extension = path.extension().string();
    if (extension.size() > 0)
    {
        extension = extension.substr(1);
    }
    else
    {
        hydra::log("No extension found for file: {}", path.string());
        return;
    }

    auto& cores = Settings::GetCoreInfo();
    std::vector<CoreInfo> available_cores;
    for (size_t i = 0; i < cores.size(); i++)
    {
        for (size_t j = 0; j < cores[i].extensions.size(); j++)
        {
            if (cores[i].extensions[j] == extension)
            {
                available_cores.push_back(cores[i]);
            }
        }
    }

    if (available_cores.size() > 1)
    {
        std::string preference = Settings::Get(extension + "_preference");
        if (!preference.empty())
        {
            for (size_t i = 0; i < available_cores.size(); i++)
            {
                if (available_cores[i].core_name == preference)
                {
                    load_rom_impl(available_cores[i].path, path);
                    return;
                }
            }

            // Preference must've been removed
            Settings::Set(extension + "_preference", "");
        }
        else
        {
            // Open a picker
            pending_rom_load = true;
            pending_rom_path = path;
            pending_available_cores = available_cores;
        }
    }
    else if (available_cores.size() == 1)
    {
        // Only 1 core available, our life is easy
        load_rom_impl(available_cores[0].path, path);
    }
    else
    {
        hydra::log("No core available for extension: {}", extension);
    }
}

void MainWindow::load_rom_impl(const std::filesystem::path& core_path,
                               const std::filesystem::path& rom_path)
{
    game_window = std::make_unique<GameWindow>(core_path, rom_path);
    if (!game_window->isLoaded())
    {
        game_window.reset();
    }
    recent_roms.push_front(rom_path);
    if (recent_roms.size() > recent_max)
    {
        recent_roms.pop_back();
    }
    save_recents();
}

void MainWindow::save_recents()
{
    for (size_t i = 0; i < recent_roms.size(); i++)
    {
        Settings::SetNoSave("recent_" + std::to_string(i), recent_roms[i].string());
    }
    for (size_t i = recent_roms.size(); i < recent_max; i++)
    {
        Settings::SetNoSave("recent_" + std::to_string(i), "");
    }
    Settings::Save();
}

void MainWindow::draw_games()
{
    if (pending_rom_load)
    {
        draw_pending_rom_load();
        return;
    }

    static bool removing_recent = false;
    bool update_positions = false;
    if (removing_recent)
    {
        static std::chrono::time_point<std::chrono::steady_clock> start =
            std::chrono::steady_clock::now();
        if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(60))
        {
            update_positions = true;
            start = std::chrono::steady_clock::now();
        }
    }
    auto [_, min, max] = draw_custom_button(ICON_MD_FOLDER " Click here to load a ROM");
    rom_picker.update(min, max, Settings::GetCoreFilters());
    int to_remove = -1;
    if (recent_roms_offsets.size() != recent_roms.size())
    {
        recent_roms_offsets.resize(recent_roms.size());
    }
    if (recent_roms.size() == 0)
    {
        return;
    }
    ImGui::Separator();
    ImGui::PushFont(big_font);
    ImGui::Text(ICON_MD_HISTORY " Recent ROMs");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - hydra::ImGuiHelper::IconWidth() * 2.0f -
                    ImGui::GetStyle().FramePadding.x * 2.0f);
    ImGui::Checkbox(ICON_MD_EDIT, &removing_recent);
    for (size_t i = 0; i < recent_roms.size(); i++)
    {
        ImGui::PushID(i);
        if (!removing_recent)
        {
            auto [clicked, min, max] = draw_custom_button(recent_roms[i].filename().string());
            if (clicked)
            {
                loadRom(recent_roms[i]);
            }
        }
        else
        {
            float shake_x = recent_roms_offsets[i].x;
            float shake_y = recent_roms_offsets[i].y;
            if (update_positions)
            {
                shake_x = (rand() % 10 - 5) / 40.0f;
                shake_y = (rand() % 10 - 5) / 40.0f;
                recent_roms_offsets[i].x = shake_x;
                recent_roms_offsets[i].y = shake_y;
            }
            ImGui::SetCursorPos(
                ImVec2(ImGui::GetCursorPos().x + shake_x, ImGui::GetCursorPos().y + shake_y));
            auto [clicked, min, max] = draw_custom_button(
                ICON_MD_DELETE " " + recent_roms[i].filename().string(), 0x40000040, 0x80000080);
            if (clicked)
            {
                to_remove = i;
            }
        }
        ImGui::PopID();
    }
    ImGui::PopFont();
    if (to_remove != -1)
    {
        recent_roms.erase(recent_roms.begin() + to_remove);
        save_recents();
    }
}

std::tuple<bool, ImVec2, ImVec2> MainWindow::draw_custom_button(const std::string& text,
                                                                uint32_t default_color,
                                                                uint32_t hover_color)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGui::PushFont(big_font);
    float text_height =
        ImGui::CalcTextSize(text.c_str(), nullptr, false,
                            ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 2)
            .y;
    ImVec2 min = ImGui::GetCursorPos();
    ImVec2 max = ImVec2(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x,
                        ImGui::GetCursorPos().y + text_height + ImGui::GetStyle().ItemSpacing.y);
    uint32_t color = default_color;
    bool hovered = ImGui::IsMouseHoveringRect(min, max) && ImGui::IsWindowHovered();
    bool clicked = false;
    if (hovered)
    {
        color = hover_color;
        clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    draw_list->AddRectFilled(min, max, color, 0, 0);
    ImVec2 min_text =
        ImVec2(min.x + ImGui::GetStyle().ItemSpacing.x, min.y + ImGui::GetStyle().ItemSpacing.y);
    ImGui::SetCursorPos(min_text);
    ImGui::TextWrapped("%s", text.c_str());
    ImGui::PopFont();
    return {clicked, min, max};
}

void MainWindow::draw_pending_rom_load()
{
    static bool remember_rom_choice = true;
    ImGui::PushFont(big_font);
    ImGui::TextWrapped(ICON_MD_WARNING " Multiple cores available for this ROM");
    ImGui::Separator();
    for (auto& core : pending_available_cores)
    {
        auto [clicked, min, max] = draw_custom_button(core.core_name + " " + core.version);
        if (clicked)
        {
            load_rom_impl(core.path, pending_rom_path);
            if (remember_rom_choice)
            {
                Settings::Set(pending_rom_path.extension().string() + "_preference",
                              core.core_name);
            }
            pending_rom_load = false;
            pending_available_cores.clear();
            pending_rom_path.clear();
            return;
        }
    }
    ImVec2 pos = ImGui::GetCursorPos();
    pos.y += ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ItemSpacing.y * 2.0f -
             hydra::ImGuiHelper::BigTextHeight();
    ImGui::SetCursorPos(pos);
    ImGui::Separator();
    ImGui::Checkbox("Remember my choice", &remember_rom_choice);
    ImGui::PopFont();
}

void MainWindow::draw_cores()
{
    auto& style = ImGui::GetStyle();

    float core_text_height =
        ImGui::CalcTextSize(HS_ADD_CORES, nullptr, false, ImGui::GetContentRegionAvail().x).y;

    ImGui::Text(ICON_MD_MEMORY " Cores");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - hydra::ImGuiHelper::IconWidth() * 2 -
                    style.FramePadding.x * 4 - style.ItemSpacing.x);
    ImGui::Button(ICON_MD_ADD);
    ImGui::SameLine();
    ImGui::Button(ICON_MD_REMOVE);
    auto& cores = Settings::GetCoreInfo();
    float widget_height = hydra::ImGuiHelper::TextHeight() * 2 + core_text_height +
                          ImGui::GetStyle().FramePadding.y * 4.0f +
                          ImGui::GetStyle().ItemSpacing.y * 3.0f + 15;
#ifdef HYDRA_WEB
    widget_height =
        ImGui::GetStyle().FramePadding.y * 2.0f + ImGui::GetStyle().ItemSpacing.y * 2.0f;
#endif
    ImGui::BeginChild("##cores", ImVec2(0, ImGui::GetWindowHeight() - widget_height),
                      ImGuiChildFlags_Border, ImGuiWindowFlags_NoScrollbar);
    float image_width = ImGui::GetWindowWidth() * 0.1f;
    image_width = std::min(image_width, 128.0f);
    image_width = std::max(image_width, 64.0f);
    for (size_t i = 0; i < cores.size(); i++)
    {
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x - 20);
        ImGui::PushID(i);
        ImGui::BeginGroup();
        float padding = 15.0f;
        ImGui::SetCursorPos(
            ImVec2(ImGui::GetCursorPos().x + padding, ImGui::GetCursorPos().y + padding));
        ImGui::Image((ImTextureID)(intptr_t)cores[i].icon_texture,
                     ImVec2(image_width, image_width));
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("%s %s (%s)\n", cores[i].core_name.c_str(), cores[i].version.c_str(),
                    cores[i].system_name.c_str());
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - style.FramePadding.x * 2 -
                        hydra::ImGuiHelper::IconWidth() - padding);
        if (ImGui::Button(ICON_MD_SETTINGS))
        {
            selected_tab = settings_tab;
            open_core_settings = i;
        }
        ImGui::Separator();
        ImGui::TextWrapped("Author: %s\n", cores[i].author.c_str());
        ImGui::Text("License: %s\n", cores[i].license.c_str());
        ImGui::Text("Description: %s\n", cores[i].description.c_str());
        ImGui::Text("Supported formats: ");
        ImGui::SameLine(0, 0);
        std::string formats;
        for (size_t j = 0; j < cores[i].extensions.size(); j++)
        {
            formats += cores[i].extensions[j];
            if (j != cores[i].extensions.size() - 1)
            {
                formats += ", ";
            }
        }
        ImGui::Text("%s\n", formats.c_str());
        ImGui::PopTextWrapPos();
        hydra::Url::draw(cores[i].url.c_str());
        ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, padding));
        ImGui::EndGroup();
        ImGui::PopID();
        ImGui::EndGroup();
        ImVec2 max = ImGui::GetItemRectMax();
        max.x = std::min(max.x, ImGui::GetContentRegionAvail().x + ImGui::GetItemRectMin().x);
        ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), max, 0x80FFFFFF, 0, 0, 0.2f);
    }
    ImGui::EndChild();

#ifndef HYDRA_WEB
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
    ImGui::TextWrapped(HS_ADD_CORES);
#endif
}

void MainWindow::draw_about() {}

void MainWindow::draw_settings()
{
    ImGui::Text(ICON_MD_SETTINGS " Settings");
    ImGui::BeginChild("##settings", ImVec2(0, 0), ImGuiChildFlags_Border,
                      ImGuiWindowFlags_NoScrollbar);
    if (ImGui::Checkbox("Enable fancy GUI", &fancy_gui))
    {
        Settings::Set("fancy_gui", fancy_gui ? "true" : "false");
    }
    ImGui::SetItemTooltip("Enable fancy GUI features that may hinder performance");

    auto& cores = Settings::GetCoreInfo();
    if (cores.size() > 0)
        ImGui::SeparatorText("Core specific settings");
    for (size_t i = 0; i < cores.size(); i++)
    {
        if (open_core_settings != -1)
        {
            if (open_core_settings == i)
            {
                ImGui::SetNextItemOpen(true);
            }
            else
            {
                ImGui::SetNextItemOpen(false);
            }
        }

        ImGui::PushID(i);
        if (ImGui::CollapsingHeader(cores[i].core_name.c_str()))
        {
            if (settings_functions.find(cores[i].core_name) != settings_functions.end())
            {
                for (auto& func : settings_functions[cores[i].core_name])
                {
                    func();
                }
            }
            else
            {
                ImGui::Text("This core has no settings");
            }
        }
        ImGui::PopID();
    }
    open_core_settings = -1;
    ImGui::EndChild();
}

void MainWindow::draw_stars(ImVec2 center, float radius)
{
    if (!fancy_gui)
    {
        ImVec2 min(center.x - radius, center.y - radius);
        ImVec2 max(center.x + radius, center.y + radius);
        ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImGui::GetColorU32(ImGuiCol_Button));
        return;
    }

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