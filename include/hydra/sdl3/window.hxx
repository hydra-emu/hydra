#pragma once

#include <SDL3/SDL.h>

namespace hydra
{
    namespace SDL3
    {
        struct Context
        {
            SDL_Window* window;
            void* inner;
        };

        namespace Vk
        {
            Context* init();
            void shutdown(Context* context);
            void startFrame(Context* context);
            void endFrame(Context* context);
        } // namespace Vk

        namespace Gl
        {
            Context* init();
            void shutdown(Context* context);
            void startFrame(Context* context);
            void endFrame(Context* context);
        } // namespace Gl

        namespace Common
        {
            enum class EventResult
            {
                Continue,
                Quit
            };

            EventResult poll(Context* context);
        } // namespace Common
    }     // namespace SDL3
} // namespace hydra