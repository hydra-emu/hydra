#include "corewrapper.hxx"
#include "hydra/core.hxx"
#include "mainwindow.hxx"
#include "SDL_events.h"
#include "SDL_render.h"
#include <cstdio>
#include <cstdlib>
#include <IconsMaterialDesign.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/imgui.h>
#include <memory>
#include <sanity.hxx>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_video.h>
#include <settings.hxx>

ImFont* small_font = nullptr;
ImFont* big_font = nullptr;

#ifdef HYDRA_WEB
#include <emscripten.h>
#include <functional>
static std::function<void()> MainLoopForEmscriptenP;

static void MainLoopForEmscripten()
{
    MainLoopForEmscriptenP();
}

#define EMSCRIPTEN_MAINLOOP_BEGIN MainLoopForEmscriptenP = [&]()
#define EMSCRIPTEN_MAINLOOP_END \
    ;                           \
    emscripten_set_main_loop(MainLoopForEmscripten, 0, true)
#else
#define EMSCRIPTEN_MAINLOOP_BEGIN
#define EMSCRIPTEN_MAINLOOP_END
#endif

extern unsigned int CourierPrime_compressed_size;
extern unsigned int CourierPrime_compressed_data[44980 / 4];
extern unsigned int MaterialIcons_compressed_size;
extern unsigned int MaterialIcons_compressed_data[246308 / 4];

int imgui_main(int argc, char* argv[])
{
    // TODO: Joystick
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

// TODO: silly example bloat, verify it's removable and remove
#if defined(IMGUI_IMPL_OPENGL_ES2)
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    uint32_t window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
    SDL_Window* window = SDL_CreateWindow("hydra", 640, 480, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_ShowWindow(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    Settings::InitCoreIcons();

    float small_size = 16.0f;
    float big_size = 40.0f;

    small_font = io.Fonts->AddFontFromMemoryCompressedTTF(CourierPrime_compressed_data,
                                                          CourierPrime_compressed_size, small_size);
    ImGui::GetIO().Fonts->Build();
    ImGui::GetIO().FontDefault = small_font;

    ImWchar icon_ranges[] = {ICON_MIN_MD, ICON_MAX_MD, 0};
    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = small_size;
    io.Fonts->AddFontFromMemoryCompressedTTF(MaterialIcons_compressed_data,
                                             MaterialIcons_compressed_size, small_size, &config,
                                             icon_ranges);

    config.GlyphMinAdvanceX = big_size;
    big_font = io.Fonts->AddFontFromMemoryCompressedTTF(CourierPrime_compressed_data,
                                                        CourierPrime_compressed_size, big_size);
    io.Fonts->AddFontFromMemoryCompressedTTF(MaterialIcons_compressed_data,
                                             MaterialIcons_compressed_size, big_size, &config,
                                             icon_ranges);

    bool done = false;
    std::unique_ptr<MainWindow> main_window = std::make_unique<MainWindow>();
#ifdef HYDRA_WEB
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    done = true;
                    break;
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    if (event.window.windowID == SDL_GetWindowID(window))
                    {
                        done = true;
                    }
                    break;
                case SDL_EVENT_DROP_FILE:
                {
                    std::filesystem::path path(event.drop.data);
                    if (path.extension().string() == hydra::dynlib_get_extension())
                    {
                        std::filesystem::path core_path = Settings::Get("core_path");
                        std::filesystem::path out_path = core_path / path.filename();
                        int copy = 1;
                        while (std::filesystem::exists(out_path))
                        {
                            out_path =
                                core_path / (path.stem().string() + " (" + std::to_string(copy) +
                                             ")" + path.extension().string());
                            copy++;
                        }
                        std::filesystem::copy(path, out_path);
                        printf("Copied %s to %s\n", path.string().c_str(),
                               out_path.string().c_str());
                    }
                    else
                    {
                        main_window->loadRom(path);
                    }
                    break;
                }
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("##Hydra", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoBringToFrontOnFocus);
        bool draw_main_window =
            true; //! game_window || (game_window && !game_window->isFullscreen());
        if (draw_main_window)
            main_window->update();
        ImGui::End();
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
#ifdef HYDRA_WEB
    EMSCRIPTEN_MAINLOOP_END;
#endif

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void Settings::InitCoreIcons()
{
    auto& cores = Settings::GetCoreInfo();
    for (size_t i = 0; i < cores.size(); i++)
    {
        if (cores[i].icon_texture)
        {
            glDeleteTextures(1, &cores[i].icon_texture);
            cores[i].icon_texture = 0;
        }

        void* handle = hydra::dynlib_open(cores[i].path.c_str());
        if (handle)
        {
            auto get_info_p =
                (decltype(hydra::getInfo)*)hydra::dynlib_get_symbol(handle, "getInfo");
            if (get_info_p)
            {
                const char* data = get_info_p(hydra::InfoType::IconData);
                if (data)
                {
                    int width = std::atoi(get_info_p(hydra::InfoType::IconWidth));
                    int height = std::atoi(get_info_p(hydra::InfoType::IconHeight));
                    if (width != 0 && height != 0)
                    {
                        glGenTextures(1, &cores[i].icon_texture);
                        glBindTexture(GL_TEXTURE_2D, cores[i].icon_texture);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                                     GL_UNSIGNED_INT_8_8_8_8, data);
                        glBindTexture(GL_TEXTURE_2D, 0);
                    }
                }
            }
        }
    }
}