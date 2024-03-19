#pragma once

#include <SDL3/SDL.h>

namespace hydra::SDL3
{

    enum class EventResult
    {
        Continue,
        Quit
    };

    namespace Renderer
    {
        void* Init();
        void Shutdown(void* context);
        void StartFrame(void* context);
        void EndFrame(void* context);
        SDL_Window* GetWindow(void* context);
    } // namespace Renderer

    namespace Common
    {
        EventResult Poll(SDL_Window* window);
    }
} // namespace hydra::SDL3