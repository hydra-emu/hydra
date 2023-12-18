#include "gamewindow.hxx"
#include <glad/glad.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <vector>

constexpr float boundary = 40.0f;
GLuint texture = 0;

GameWindow::GameWindow()
{
    ImGuiIO& io = ImGui::GetIO();
    size_x = io.DisplaySize.x / 3.0f;
    size_y = io.DisplaySize.y / 4.0f;
    snap_x = io.DisplaySize.x - size_x - boundary;
    snap_y = io.DisplaySize.y - size_y - boundary;

    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureStorage2D(texture, 1, GL_RGBA8, 256, 256);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // fill texture with red gradient
    std::vector<uint32_t> data(256 * 256);
    for (int y = 0; y < 256; y++)
    {
        for (int x = 0; x < 256; x++)
        {
            data[y * 256 + x] = 0xFF000000 | (x << 16) | (y << 8);
        }
    }
    glTextureSubImage2D(texture, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
}

void GameWindow::update()
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGuiIO& io = ImGui::GetIO();

    if (!fullscreen || animating)
        ImGui::GetStyle().WindowRounding = 10.0f;

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
            if (!snapped)
            {
                if (velocity_x > 1.0f)
                {
                    snap_x = io.DisplaySize.x - size_x - boundary;
                    snapped = true;
                }
                else if (velocity_x < -1.0f)
                {
                    snap_x = boundary;
                    snapped = true;
                }
                if (velocity_y > 1.0f)
                {
                    snap_y = io.DisplaySize.y - size_y - boundary;
                    snapped = true;
                }
                else if (velocity_y < -1.0f)
                {
                    snap_y = boundary;
                    snapped = true;
                }
                if (!snapped)
                {
                    float center_x = position_x + size_x / 2.0f;
                    float center_y = position_y + size_y / 2.0f;
                    if (center_x < io.DisplaySize.x / 2.0f)
                    {
                        snap_x = boundary;
                    }
                    else
                    {
                        snap_x = io.DisplaySize.x - size_x - boundary;
                    }
                    if (center_y < io.DisplaySize.y / 2.0f)
                    {
                        snap_y = boundary;
                    }
                    else
                    {
                        snap_y = io.DisplaySize.y - size_y - boundary;
                    }
                    snapped = true;
                }
            }
            position_x += (snap_x - position_x) / 10.0f;
            position_y += (snap_y - position_y) / 10.0f;
        }
    }
    if (!animating)
    {
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
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
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
    if (ImGui::Button("Fullscreen") && !animating)
    {
        fullscreen ^= true;
        animating = true;
    }
    // ImGui::Image((ImTextureID)&texture, ImVec2(256, 256),
    //              ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
    ImGui::PopStyleColor();
}