#include "compatibility.hxx"
#include "corewrapper.hxx"
#include "filepicker.hxx"
#include "hydra/core.hxx"
#include "mainwindow.hxx"
#include "SDL_events.h"
#include "SDL_render.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <glad/glad.h>
#include <IconsMaterialDesign.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/imgui.h>
#include <map>
#include <memory>
#include <sanity.hxx>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <settings.hxx>
#include <toml11/toml.hpp>
#include <unordered_map>
#include <vector>

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

std::unique_ptr<MainWindow> main_window;

extern "C" void hydra_drop_file(const char* name, void* data, size_t size)
{
    std::filesystem::path file = name;
    if (file.extension() == hydra::dynlib_get_extension())
    {
        std::filesystem::path cores = Settings::Get("core_path");
        std::filesystem::path out_path = cores / file;
        std::ofstream ofs(out_path, std::ios_base::binary | std::ios_base::out);
        ofs.write((const char*)data, size);
        sync_fs([] { Settings::ReinitCoreInfo(); });
    }
    else
    {
        main_window->loadRom(file);
    }
}

extern "C" void callback_wrapper(void* callback)
{
    if (callback)
    {
        std::function<void()>* func = (std::function<void()>*)callback;
        (*func)();
        delete func;
    }
}

#ifdef HYDRA_WEB
EM_JS(void, hydra_web_initialize, (), {
          var canvas = document.getElementById("canvas");
          canvas.addEventListener(
              "dragenter",
              function(e) {
                  e.preventDefault();
                  e.stopPropagation();
              },
              false);
          canvas.addEventListener(
              "dragover",
              function(e) {
                  e.preventDefault();
                  e.stopPropagation();
              },
              false);
          canvas.addEventListener(
              "dragleave",
              function(e) {
                  e.preventDefault();
                  e.stopPropagation();
              },
              false);
          canvas.addEventListener(
              "drop",
              function(e) {
                  e.preventDefault();
                  e.stopPropagation();
                  var files = e.dataTransfer.files;
                  for (var i = 0, f; f = files[i]; i++)
                  {
                      var reader = new FileReader();
                      reader.onload = (function(file) {
                          return function(e)
                          {
                              var ptr = Module._malloc(file.size);
                              Module.HEAPU8.set(new Uint8Array(e.target.result), ptr);
                              Module.ccall('hydra_drop_file', 'void',
                                           [ 'string', 'number', 'number' ],
                                           [ file.name, ptr, file.size ]);
                              Module._free(ptr);
                          };
                      })(f);
                      reader.readAsArrayBuffer(f);
                  }
              },
              false);

});
#endif

#ifdef HYDRA_DESKTOP
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                GLsizei length, const GLchar* message, const void* userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}
#endif

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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    uint32_t window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
    SDL_Window* window = SDL_CreateWindow("hydra", 1024, 768, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_ShowWindow(window);

    glGetString = (PFNGLGETSTRINGPROC)SDL_GL_GetProcAddress("glGetString");
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

#if !defined(HYDRA_WEB) && !defined(HYDRA_ANDROID) && !defined(HYDRA_IOS)
    if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0)
    {
        printf("Failed to initialize OpenGL functions\n");
        return 1;
    }
#else
    if (gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress) == 0)
    {
        printf("Failed to initialize OpenGL functions!\n");
        return 1;
    }
#endif

#ifdef HYDRA_DESKTOP
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);
#endif

#ifdef HYDRA_WEB
    hydra_web_initialize();
#endif

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    main_window = std::make_unique<MainWindow>();
    Settings::ReinitCoresFrontend();

    float small_size = 16.0f;
    float big_size = 40.0f;

    small_font = io.Fonts->AddFontFromMemoryCompressedTTF(CourierPrime_compressed_data,
                                                          CourierPrime_compressed_size, small_size);
    io.Fonts->Build();
    io.FontDefault = small_font;

    ImWchar icon_ranges[] = {ICON_MIN_MD, ICON_MAX_MD, 0};
    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = small_size;
    config.GlyphOffset.y = small_size / 4;
    io.Fonts->AddFontFromMemoryCompressedTTF(MaterialIcons_compressed_data,
                                             MaterialIcons_compressed_size, small_size, &config,
                                             icon_ranges);

    config.GlyphMinAdvanceX = big_size;
    config.GlyphOffset.y = big_size / 4;
    big_font = io.Fonts->AddFontFromMemoryCompressedTTF(CourierPrime_compressed_data,
                                                        CourierPrime_compressed_size, big_size);
    io.Fonts->AddFontFromMemoryCompressedTTF(MaterialIcons_compressed_data,
                                             MaterialIcons_compressed_size, big_size, &config,
                                             icon_ranges);

    bool done = false;
#ifdef HYDRA_WEB
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
                        Settings::ReinitCoreInfo();
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
                         ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoScrollWithMouse);
        FilePicker::hideAll();
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

void Settings::ReinitCoresFrontend()
{
    auto& cores = Settings::GetCoreInfo();
    std::unordered_map<std::string, std::vector<std::function<void()>>> functions;
    for (size_t i = 0; i < cores.size(); i++)
    {
        if (cores[i].icon_texture)
        {
            glDeleteTextures(1, &cores[i].icon_texture);
            cores[i].icon_texture = 0;
        }
        cores[i].required_files.clear();

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
                                     GL_UNSIGNED_BYTE, data);
                        glBindTexture(GL_TEXTURE_2D, 0);
                    }
                }

                const char* toml = get_info_p(hydra::InfoType::Settings);
                if (toml && std::string(toml).size() > 0)
                {
                    struct Widget
                    {
                        std::string key;
                        std::string name;
                        std::string description;
                        std::string category;
                        std::string type;
                        std::string default_;
                        bool required = false;
                    };

                    std::string toml_s(toml);
                    std::istringstream ifs(toml_s, std::ios_base::binary | std::ios_base::in);
                    toml::value toml11 = toml::parse(ifs);
                    if (!toml11.is_table())
                    {
                        printf("Error: Settings for core %s is not a table\n",
                               cores[i].core_name.c_str());
                        continue;
                    }

                    std::map<std::string, std::vector<Widget>> widgets;
                    for (const auto& [table_name, table] : toml11.as_table())
                    {
                        std::string setting_key = cores[i].core_name + "_" + table_name;
                        Widget widget;
                        widget.key = setting_key;
                        for (const auto& [key, value] : table.as_table())
                        {
#define cs(x)                                                       \
    if (!value.is_##x())                                            \
    {                                                               \
        printf("Error: Setting %s is not a %s\n", key.c_str(), #x); \
        continue;                                                   \
    }
                            switch (hydra::str_hash(key.c_str()))
                            {
                                case hydra::str_hash("name"):
                                    cs(string) widget.name = value.as_string();
                                    break;
                                case hydra::str_hash("description"):
                                    cs(string) widget.description = value.as_string();
                                    break;
                                case hydra::str_hash("category"):
                                    cs(string) widget.category = value.as_string();
                                    break;
                                case hydra::str_hash("type"):
                                    cs(string) widget.type = value.as_string();
                                    break;
                                case hydra::str_hash("default"):
                                    if (value.is_boolean())
                                    {
                                        widget.default_ = value.as_boolean() ? "true" : "false";
                                    }
                                    else if (value.is_integer())
                                    {
                                        widget.default_ = std::to_string(value.as_integer());
                                    }
                                    else if (value.is_floating())
                                    {
                                        widget.default_ = std::to_string(value.as_floating());
                                    }
                                    else if (value.is_string())
                                    {
                                        widget.default_ = value.as_string();
                                    }
                                    else
                                    {
                                        printf("Error: Setting %s is not a valid type\n",
                                               key.c_str());
                                        continue;
                                    }
                                    break;
                                case hydra::str_hash("required"):
                                    cs(boolean) widget.required = value.as_boolean();
                                    break;
                            }
#undef cs
                        }

                        if (widget.required)
                        {
                            cores[i].required_files.push_back(table_name);
                        }

                        if (widget.category.empty())
                        {
                            widgets["#__root__"].push_back(widget);
                        }
                        else
                        {
                            widgets[widget.category].push_back(widget);
                        }
                    }

                    std::string core_name = cores[i].core_name;
                    auto& core_functions = functions[core_name];
                    int widget_id = 0;
                    for (const auto& [category, widget_list] : widgets)
                    {
                        std::string category_string = category;
                        if (category_string != "#__root__")
                        {
                            core_functions.push_back([category_string]() {
                                ImGui::Text("%s", category_string.c_str());
                                ImGui::Separator();
                                ImGui::Indent();
                            });
                        }

                        for (const auto& widget : widget_list)
                        {
                            if (!widget.default_.empty() && Settings::Get(widget.key).empty())
                            {
                                Settings::Set(widget.key, widget.default_);
                            }
                            int current_id = widget_id++;
                            std::string name = widget.name;
                            std::string key = widget.key;
                            std::string description = widget.description;
                            switch (hydra::str_hash(widget.type))
                            {
                                case hydra::str_hash("checkbox"):
                                {
                                    bool value = Settings::Get(key) == "true";
                                    core_functions.push_back(
                                        [name, key, description, value, current_id]() mutable {
                                            ImGui::PushID(current_id);
                                            if (ImGui::Checkbox(name.c_str(), &value))
                                            {
                                                Settings::Set(key, value ? "true" : "false");
                                            }
                                            if (!description.empty())
                                                ImGui::SetItemTooltip("%s", description.c_str());
                                            ImGui::PopID();
                                        });
                                    break;
                                }
                                case hydra::str_hash("filepicker"):
                                {
                                    std::string value = Settings::Get(widget.key);
                                    value.resize(1024);
                                    std::shared_ptr<FilePicker> picker;
                                    core_functions.push_back([name, key, description, value,
                                                              current_id, picker]() mutable {
                                        if (!picker)
                                        {
                                            picker = std::make_shared<FilePicker>(
                                                key, name, [key, &value](const char* path) {
                                                    value = path;
                                                    Settings::Set(key, path);
                                                });
                                        }
                                        ImGui::PushID(current_id);
                                        ImGui::InputText(name.c_str(), value.data(), value.size(),
                                                         ImGuiInputTextFlags_ReadOnly);
                                        if (!description.empty())
                                            ImGui::SetItemTooltip("%s", description.c_str());
                                        ImGui::SameLine();
                                        if (ImGui::Button(ICON_MD_FOLDER " Browse"))
                                        {
                                            std::string path = hydra::file_picker();
                                            if (!path.empty())
                                            {
                                                Settings::Set(key, path);
                                                if (path.size() > 1024)
                                                {
                                                    value.resize(path.size());
                                                }
                                                std::memcpy(value.data(), path.data(), path.size());
                                            }
                                        }
                                        picker->update(ImGui::GetItemRectMin(),
                                                       ImGui::GetItemRectMax(),
                                                       {{"All files", ".*"}});
                                        ImGui::PopID();
                                    });
                                    break;
                                }
                            }
                        }

                        if (category != "#__root__")
                        {
                            core_functions.push_back([]() { ImGui::Unindent(); });
                        }
                    }
                }
            }
            hydra::dynlib_close(handle);
        }
    }
    main_window->setSettingsFunctions(functions);
}