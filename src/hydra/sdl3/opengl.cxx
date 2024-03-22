#include "backends/imgui_impl_sdl3.h"
#include "hydra/imgui/imgui.hxx"

#include <glad/glad.h>
#include <SDL3/SDL.h>

#include <hydra/common/log.hxx>
#include <hydra/common/version.hxx>
#include <hydra/core.h>
#include <hydra/sdl3/window.hxx>

struct InnerContext
{
    SDL_GLContext glContext;
};

namespace hydra::SDL3::Gl
{

    Context* init(const HcEnvironmentInfo* environmentInfo)
    {
        Context* ctx = new Context();
        InnerContext* inner = new InnerContext();
        ctx->inner = inner;

        if (!environmentInfo || !environmentInfo->video)
        {
            hydra::panic("Invalid environment info");
            return nullptr;
        }

        if (environmentInfo->video->rendererType != HC_RENDERER_TYPE_OPENGL)
        {
            hydra::panic("Expected OpenGL renderer type");
            return nullptr;
        }

        int major = 0, minor = 0;
        switch (environmentInfo->video->rendererVersion)
        {
            case HC_OPENGL_VERSION_1_0:
            case HC_OPENGL_VERSION_1_1:
            case HC_OPENGL_VERSION_1_2:
            case HC_OPENGL_VERSION_1_3:
            case HC_OPENGL_VERSION_1_4:
            case HC_OPENGL_VERSION_1_5:
            case HC_OPENGL_VERSION_2_0:
            case HC_OPENGL_VERSION_2_1:
            case HC_OPENGL_VERSION_3_0:
            case HC_OPENGL_VERSION_3_1:
            case HC_OPENGL_VERSION_3_2:
            case HC_OPENGL_VERSION_3_3:
            case HC_OPENGL_VERSION_4_0:
            case HC_OPENGL_VERSION_4_1:
            case HC_OPENGL_VERSION_4_2:
            case HC_OPENGL_VERSION_4_3:
            case HC_OPENGL_VERSION_4_4:
            case HC_OPENGL_VERSION_4_5:
            case HC_OPENGL_VERSION_4_6:
                major = (int)environmentInfo->video->rendererVersion >> 16;
                minor = (int)environmentInfo->video->rendererVersion & 0xFFFF;
                break;
            default:
                hydra::panic("Unknown OpenGL version: {}",
                             (int)environmentInfo->video->rendererVersion);
                return nullptr;
        }

        switch (environmentInfo->video->format)
        {
            case HC_PIXEL_FORMAT_ABGR32:
            case HC_PIXEL_FORMAT_ARGB32:
            case HC_PIXEL_FORMAT_BGRA32:
            case HC_PIXEL_FORMAT_RGBA32:
                SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
                break;
            case HC_PIXEL_FORMAT_RGB24:
            case HC_PIXEL_FORMAT_BGR24:
                SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
                break;
            case HC_PIXEL_FORMAT_RGB565:
                SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
                SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
                SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
                break;
            case HC_PIXEL_FORMAT_RGBA5551:
            case HC_PIXEL_FORMAT_ARGB1555:
                SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
                SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
                SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
                break;
            default:
                hydra::panic("Unknown pixel format: {}", (int)environmentInfo->video->format);
                return nullptr;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        int width = environmentInfo->video->width;
        int height = environmentInfo->video->height;

        uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
        ctx->window = SDL_CreateWindow(hydra::common::version().c_str(), width, height, flags);

        SDL_SetWindowPosition(ctx->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

        inner->glContext = SDL_GL_CreateContext(ctx->window);
        if (!inner->glContext)
        {
            hydra::panic("Failed to create OpenGL context: {}", SDL_GetError());
            return nullptr;
        }

        if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0)
        {
            hydra::panic("Failed to load OpenGL functions: {}", SDL_GetError());
            return nullptr;
        }

        hydra::imgui::init();

        ImGui_ImplSDL3_InitForOpenGL(ctx->window, inner->glContext);

        return ctx;
    }

    void present(Context* context)
    {
        printf("TODO: present");
    }

    void shutdown(Context* context)
    {
        if (!context || !context->window)
        {
            hydra::panic("Invalid context");
            return;
        }

        ImGui_ImplSDL3_Shutdown();
        hydra::imgui::shutdown();

        InnerContext* inner = (InnerContext*)context->inner;
        if (!inner || !inner->glContext)
        {
            hydra::panic("Invalid inner context");
            return;
        }
        else
        {
            SDL_GL_DeleteContext(inner->glContext);
        }

        if (context->window)
        {
            SDL_DestroyWindow(context->window);
        }

        delete inner;
        delete context;
    }

} // namespace hydra::SDL3::Gl