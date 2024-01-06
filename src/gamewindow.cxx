#include "gamewindow.hxx"
#include "IconsMaterialDesign.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <SDL3/SDL_opengl.h>
#include <stb_image.h>
#include <test_image.h>
#include <vector>

constexpr float boundary = 40.0f;
constexpr float rounding = 10.0f;
extern ImFont* big_font;

GameWindow::GameWindow()
{
    ImGuiIO& io = ImGui::GetIO();
    size_x = io.DisplaySize.x / 3.0f;
    size_y = io.DisplaySize.y / 4.0f;
    snap_x = io.DisplaySize.x - size_x - boundary;
    snap_y = io.DisplaySize.y - size_y - boundary;

    std::vector<uint8_t> pixels(240 * 160 * 4);
    int x, y, z;
    auto image = stbi_load_from_memory(screen, screen_size, &x, &y, &z, 4);
    memcpy(pixels.data(), image, 240 * 160 * 4);
    stbi_image_free(image);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 240, 160, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GameWindow::update()
{
    // if (!emulator)
    //     return;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGuiIO& io = ImGui::GetIO();

    if (!fullscreen || animating)
        ImGui::GetStyle().WindowRounding = rounding;

    if (!fullscreen)
    {
        if (held)
        {
            velocity_x = io.MouseDelta.x;
            velocity_y = io.MouseDelta.y;
            if (velocity_x > 20.0f)
                velocity_x = 20.0f;
            else if (velocity_x < -20.0f)
                velocity_x = -20.0f;
            if (velocity_y > 20.0f)
                velocity_y = 20.0f;
            else if (velocity_y < -20.0f)
                velocity_y = -20.0f;
            position_x += velocity_x;
            position_y += velocity_y;
            snapped = false;
        }
        else
        {
            if (velocity_x > 1.0f)
            {
                horizontal_snap = 1;
                velocity_x = 0.9f;
            }
            else if (velocity_x < -1.0f)
            {
                horizontal_snap = -1;
                velocity_x = -0.9f;
            }
            if (velocity_y > 1.0f)
            {
                vertical_snap = 1;
                velocity_y = 0.9f;
            }
            else if (velocity_y < -1.0f)
            {
                vertical_snap = -1;
                velocity_y = -0.9f;
            }

            if (horizontal_snap == -1)
            {
                snap_x = boundary;
            }
            else if (horizontal_snap == 1)
            {
                snap_x = io.DisplaySize.x - size_x - boundary;
            }
            if (vertical_snap == -1)
            {
                snap_y = boundary;
            }
            else if (vertical_snap == 1)
            {
                snap_y = io.DisplaySize.y - size_y - boundary;
            }
            position_x += (snap_x - position_x) / 10.0f;
            position_y += (snap_y - position_y) / 10.0f;
        }
    }
    if (!animating)
    {
        size_x = io.DisplaySize.x / 3.0f;
        size_y = io.DisplaySize.y / 4.0f;
        if (!fullscreen)
        {
            ImGui::SetNextWindowPos(ImVec2(position_x, position_y));
            ImGui::SetNextWindowSize(ImVec2(size_x, size_y));
        }
        else
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(io.DisplaySize);
        }
    }
    else
    {
        float target_x;
        float target_y;
        float target_size_x;
        float target_size_y;
        if (!fullscreen)
        {
            target_x = snap_x;
            target_y = snap_y;
            target_size_x = io.DisplaySize.x / 3.0f;
            target_size_y = io.DisplaySize.y / 4.0f;
        }
        else
        {
            target_x = 0;
            target_y = 0;
            target_size_x = io.DisplaySize.x;
            target_size_y = io.DisplaySize.y;
        }
        position_x += (target_x - position_x) / 10.0f;
        position_y += (target_y - position_y) / 10.0f;
        size_x += (target_size_x - size_x) / 10.0f;
        size_y += (target_size_y - size_y) / 10.0f;
        if (position_x - target_x < 0.3f && position_y - target_y < 0.3f &&
            size_x - target_size_x < 0.3f && size_y - target_size_y < 0.3f)
        {
            animating = false;
        }
        ImGui::SetNextWindowPos(ImVec2(position_x, position_y));
        ImGui::SetNextWindowSize(ImVec2(size_x, size_y));
    }
    ImGui::Begin("Game", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (!fullscreen && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) &&
        ImGui::IsWindowHovered())
    {
        fullscreen = true;
        animating = true;
    }
    float actual_x = ImGui::GetWindowPos().x;
    float actual_y = ImGui::GetWindowPos().y;
    float mouse_x = io.MousePos.x;
    float mouse_y = io.MousePos.y;
    bool hovering = ImGui::IsWindowHovered();
    bool clicked = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    if (hovering && clicked)
    {
        held = true;
    }
    else
    {
        held = false;
    }

    static bool fullscreen_hovered = false;
    float ratio = 160.0f / 240.0f;
    float width = ImGui::GetWindowWidth();
    float height = ratio * width;
    float offset_x = 0;
    float offset_y = 0;

    if (height > ImGui::GetWindowHeight())
    {
        height = ImGui::GetWindowHeight();
        width = height / ratio;
        offset_x = (ImGui::GetWindowWidth() - width) / 2.0f;
    }
    else
    {
        offset_y = (ImGui::GetWindowHeight() - height) / 2.0f;
    }

    if (!fullscreen)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        offset_x += ImGui::GetWindowPos().x;
        offset_y += ImGui::GetWindowPos().y;
        draw_list->AddImageRounded((ImTextureID)(intptr_t)texture, ImVec2(offset_x, offset_y),
                                   ImVec2(offset_x + width, offset_y + height), ImVec2(0, 0),
                                   ImVec2(1, 1), ImColor(255, 255, 255, 255), rounding);
    }
    else
    {
        ImGui::SetCursorPos(ImVec2(offset_x, offset_y));
        ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(width, height));
    }
    ImGui::SetCursorPos(ImVec2(16, 16));
    ImGui::PushFont(big_font);
    if (fullscreen_hovered)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 1, 0.75));
    ImGui::Text(ICON_MD_FULLSCREEN);
    if (ImGui::IsItemClicked())
    {
        fullscreen ^= true;
        animating = true;
    }
    if (fullscreen_hovered)
        ImGui::PopStyleColor();
    fullscreen_hovered = ImGui::IsItemHovered();
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleColor();
}