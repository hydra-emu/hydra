#include "backends/imgui_impl_sdl3.h"
#include <filesystem>
#include <sdl3.hxx>
#include <SDL3/SDL.h>
#include <settings.hxx>

namespace hydra::SDL3::Common
{
    EventResult Poll(SDL_Window* window)
    {
        EventResult result = EventResult::Continue;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                {
                    result = EventResult::Quit;
                    break;
                }

                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                {
                    if (event.window.windowID == SDL_GetWindowID(window))
                    {
                        result = EventResult::Quit;
                    }
                    break;
                }

                case SDL_EVENT_DROP_FILE:
                {
                    std::filesystem::path path(event.drop.data);
                    if (path.extension().string() == hydra::dynlib_get_extension())
                    {
                        std::filesystem::path core_path = Settings::Get("core_path");
                        std::filesystem::path out_path = core_path / path.filename();
                        int copy = 1;
                        // TODO: we should replace cores instead (after prompting user)
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
                        // hydra::MainWindow::loadRom(path.string());
                    }
                    break;
                }
            }
        }

        return result;
    }
} // namespace hydra::SDL3::Common