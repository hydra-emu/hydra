#pragma once

#include <hydra/core.h>
#include <SDL3/SDL.h>

namespace hydra
{
    namespace SDL3
    {
        struct Context
        {
            HcRendererType rendererType;
            SDL_Window* window;
            void* inner;
        };

        Context* init(const HcEnvironmentInfo* createInfo);
        void present(Context* context);
        void shutdown(Context* context);

        namespace Vk
        {
            Context* init(const HcEnvironmentInfo* createInfo);
            void present(Context* context);
            void shutdown(Context* context);
        } // namespace Vk

        namespace Gl
        {
            Context* init(const HcEnvironmentInfo* createInfo);
            void present(Context* context);
            void shutdown(Context* context);
        } // namespace Gl

        enum class EventResult
        {
            Continue,
            Quit
        };

        EventResult poll(Context* context);
    } // namespace SDL3
} // namespace hydra