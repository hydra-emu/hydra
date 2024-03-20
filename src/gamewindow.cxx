#include "gamewindow.hxx"
#include "corewrapper.hxx"
#include "glad/glad.h"
#include "hydra/core.hxx"
#include "IconsMaterialDesign.h"
#include "input.hxx"
#include "log.hxx"
#include <fstream>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui_helper.hxx>
#include <stb_image.h>
#include <vector>

constexpr float boundary = 40.0f;
constexpr float rounding = 10.0f;
extern ImFont* big_font;
GameWindow* GameWindow::instance = nullptr;

GameWindow::GameWindow(const std::filesystem::path& core_path,
                       const std::filesystem::path& game_path)
{
    ImGuiIO& io = ImGui::GetIO();
    size_x = io.DisplaySize.x / 3.0f;
    size_y = io.DisplaySize.y / 4.0f;
    snap_x = io.DisplaySize.x - size_x - boundary;
    snap_y = io.DisplaySize.y - size_y - boundary;

    GameWindow::instance = this;
    emulator = hydra::EmulatorFactory::Create(core_path);
    texture_size = emulator->shell->getNativeSize();
    gl_format = GL_RGBA;

    pixel_format = emulator->GetPixelFormat();
    gl_format = get_gl_format(pixel_format);
    emulator->SetVideoCallback(video_callback);

    if (emulator->shell->hasInterface(hydra::InterfaceType::IInput))
    {
        hydra::IInput* shell_input = emulator->shell->asIInput();
        shell_input->setPollInputCallback(&hydra::Input::Poll);
        shell_input->setCheckButtonCallback(&hydra::Input::Check);
        printf("TODO: remove hardcoded input associate\n");
        hydra::Input::Associate(0, "Default mappings");
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_size.width, texture_size.height, 0, gl_format,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Framebuffer error: %d\n", status);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    emulator->SetGetProcAddress((void*)SDL_GL_GetProcAddress);
    emulator->ResetContext();
    emulator->SetFbo(fbo);
    auto error = glGetError();
    if (error != GL_NO_ERROR)
    {
        hydra::log("OpenGL error during rom load: {:x}\n", error);
        loaded = false;
        return;
    }

    loaded = emulator->LoadGame(game_path.string());
}

void GameWindow::platform_specific_changes(void* data, hydra::Size size)
{
    // TODO: do this with shaders?
#ifdef HYDRA_WEB
    if (GameWindow::instance->pixel_format == hydra::PixelFormat::BGRA)
    {
        for (int i = 0; i < size.width * size.height * 4; i += 4)
        {
            std::swap(((uint8_t*)data)[i], ((uint8_t*)data)[i + 2]);
        }
    }
#endif
}

void GameWindow::video_callback(void* data, hydra::Size size)
{
    platform_specific_changes(data, size);
    glBindTexture(GL_TEXTURE_2D, GameWindow::instance->texture);
    hydra::Size texture_size = GameWindow::instance->texture_size;
    hydra::PixelFormat pixel_format =
        GameWindow::instance->emulator->shell->asISoftwareRendered()->getPixelFormat();
    if (size.width != texture_size.width || size.height != texture_size.height ||
        pixel_format != GameWindow::instance->pixel_format)
    {
        GameWindow::instance->texture_size = size;
        GameWindow::instance->pixel_format = pixel_format;
        GameWindow::instance->gl_format = get_gl_format(pixel_format);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width, size.height, 0,
                     GameWindow::instance->gl_format, GL_UNSIGNED_BYTE, nullptr);
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.width, size.height,
                    GameWindow::instance->gl_format, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

UpdateResult GameWindow::update()
{
    bool flip_y = false;
    if (emulator->shell->hasInterface(hydra::InterfaceType::IOpenGlRendered))
    {
        hydra::IOpenGlRendered* shell_gl = emulator->shell->asIOpenGlRendered();
        shell_gl->setFbo(fbo);
        flip_y = true; // TODO: I HATE OPENGL
    }
    emulator->RunFrame();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // TODO: REMOVE

    UpdateResult result = UpdateResult::None;
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
        if (!held)
        {
            start_drag = ImVec2(mouse_x, mouse_y);
            held = true;
        }
    }
    else
    {
        if (held)
        {
            held = false;
            ImVec2 end_drag = ImVec2(mouse_x, mouse_y);
            float minimum_x = 0.1f * io.DisplaySize.x;
            float minimum_y = 0.1f * io.DisplaySize.y;
            if (end_drag.x - start_drag.x > minimum_x)
            {
                horizontal_snap = 1;
            }
            else if (end_drag.x - start_drag.x < -minimum_x)
            {
                horizontal_snap = -1;
            }
            if (end_drag.y - start_drag.y > minimum_y)
            {
                vertical_snap = 1;
            }
            else if (end_drag.y - start_drag.y < -minimum_y)
            {
                vertical_snap = -1;
            }
        }
    }

    static bool fullscreen_hovered = false;
    static bool exit_hovered = false;
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

    ImVec2 uv_min = ImVec2(0.0f, 0.0f);
    ImVec2 uv_max = ImVec2(1.0f, 1.0f);
    if (flip_y)
    {
        uv_min.y = 1.0f;
        uv_max.y = 0.0f;
    }

    // We don't want transparent parts of the texture to be transparent in the window
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddCallback(
        [](const ImDrawList* parent_list, const ImDrawCmd* cmd) { glDisable(GL_BLEND); }, nullptr);
    if (!fullscreen)
    {
        offset_x += ImGui::GetWindowPos().x;
        offset_y += ImGui::GetWindowPos().y;
        draw_list->AddImageRounded((ImTextureID)(intptr_t)texture, ImVec2(offset_x, offset_y),
                                   ImVec2(offset_x + width, offset_y + height), uv_min, uv_max,
                                   ImColor(255, 255, 255, 255), rounding);
    }
    else
    {
        ImGui::SetCursorPos(ImVec2(offset_x, offset_y));
        ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(width, height), uv_min, uv_max);
    }
    draw_list->AddCallback(
        [](const ImDrawList* parent_list, const ImDrawCmd* cmd) { glEnable(GL_BLEND); }, nullptr);
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
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 16 - hydra::ImGuiHelper::IconWidth(),
                               16)); // TODO: global icon width value
    if (exit_hovered)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 0.75));
    ImGui::Text(ICON_MD_EXIT_TO_APP);
    if (ImGui::IsItemClicked())
        result = UpdateResult::Quit;
    if (exit_hovered)
        ImGui::PopStyleColor();
    exit_hovered = ImGui::IsItemHovered();
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleColor();
    return result;
}

GLenum GameWindow::get_gl_format(hydra::PixelFormat format)
{
    switch (format)
    {
        case hydra::PixelFormat::RGBA:
        {
            return GL_RGBA;
        }
        case hydra::PixelFormat::BGRA:
        {
#ifdef HYDRA_WEB
            return GL_RGBA;
#else
            return GL_BGRA;
#endif
        }
        default:
        {
            printf("Unsupported pixel format\n");
            return GL_RGBA;
        }
    }
}