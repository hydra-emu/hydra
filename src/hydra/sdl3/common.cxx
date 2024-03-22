#include "backends/imgui_impl_sdl3.h"
#include <filesystem>
#include <hydra/common/log.hxx>
#include <hydra/sdl3/window.hxx>
#include <SDL3/SDL.h>

namespace hydra::SDL3
{
    Context* init(const HcEnvironmentInfo* environmentInfo)
    {
        if (!environmentInfo || !environmentInfo->video)
        {
            hydra::panic("Invalid environment info");
            return nullptr;
        }

        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            hydra::panic("SDL_Init: {}", SDL_GetError());
        }

        SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

        switch (environmentInfo->video->rendererType)
        {
            case HC_RENDERER_TYPE_OPENGL:
            {
                return Gl::init(environmentInfo);
            }
            case HC_RENDERER_TYPE_VULKAN:
            {
                return Vk::init(environmentInfo);
            }
            default:
            {
                hydra::panic("Unknown renderer type: {}",
                             (int)environmentInfo->video->rendererType);
                return nullptr;
            }
        }
    }

    void present(Context* context)
    {
        if (!context || !context->window)
        {
            hydra::panic("Invalid context");
            return;
        }

        switch (context->rendererType)
        {
            case HC_RENDERER_TYPE_OPENGL:
            {
                Gl::present(context);
                break;
            }
            case HC_RENDERER_TYPE_VULKAN:
            {
                Vk::present(context);
                break;
            }
            default:
            {
                hydra::panic("Unknown renderer type: {}", (int)context->rendererType);
            }
        }
    }

    void shutdown(Context* context)
    {
        if (!context || !context->window)
        {
            hydra::panic("Invalid context");
            return;
        }

        switch (context->rendererType)
        {
            case HC_RENDERER_TYPE_OPENGL:
            {
                Gl::shutdown(context);
                break;
            }
            case HC_RENDERER_TYPE_VULKAN:
            {
                Vk::shutdown(context);
                break;
            }
            default:
            {
                hydra::panic("Unknown renderer type: {}", (int)context->rendererType);
            }
        }
    }

    EventResult poll(Context* ctx)
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
                    if (event.window.windowID == SDL_GetWindowID(ctx->window))
                    {
                        result = EventResult::Quit;
                    }
                    break;
                }

                case SDL_EVENT_DROP_FILE:
                {
                    std::filesystem::path path(event.drop.data);
                    // if (path.extension().string() == hydra::dynlib_get_extension())
                    {
                        printf("FIXME\n");
                        std::filesystem::path core_path; // = Settings::Get("core_path");
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
                        // Settings::ReinitCoreInfo();
                        printf("Copied %s to %s\n", path.string().c_str(),
                               out_path.string().c_str());
                    }
                    // else
                    {
                        // hydra::MainWindow::loadRom(path.string());
                    }
                    break;
                }
            }
        }

        return result;
    }
} // namespace hydra::SDL3