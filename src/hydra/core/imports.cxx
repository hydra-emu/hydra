// Define the hydra core API imports

#include "SDL_video.h"
#include <cstdio>
#include <cstring>
#include <hydra/common/log.hxx>
#include <hydra/common/system.hxx>
#include <hydra/common/utility.hxx>
#include <hydra/core.h>
#include <thread>

#include <glad/glad.h>
#include <SDL3/SDL.h>

#include <vulkan/vulkan.h>

static HcHostInfo* globalHostInfo = nullptr;

extern "C" {

void hydra_hcGetHostInfo(HcHostInfo* hostInfo)
{
    if (!globalHostInfo)
    {
        globalHostInfo = new HcHostInfo();
        globalHostInfo->architecture = hydra::common::architecture();
        globalHostInfo->operatingSystem = hydra::common::operatingSystem();

        // Create a hidden OpenGL window to get the OpenGL version
        std::thread context([]() {
            SDL_Window* window =
                SDL_CreateWindow("hidden", 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
            SDL_GLContext context = SDL_GL_CreateContext(window);
            if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0)
            {
                hydra::log("Failed to load OpenGL functions when trying to get OpenGL version");
                globalHostInfo->openGlVersion = HC_OPENGL_NOT_SUPPORTED;
            }
            else
            {
                int major, minor;
                glGetIntegerv(GL_MAJOR_VERSION, &major);
                glGetIntegerv(GL_MINOR_VERSION, &minor);
                globalHostInfo->openGlVersion = (HcOpenGlVersion)((major << 16) | minor);
                printf("OpenGL version: %d.%d\n", major, minor);
            }

            SDL_GL_DeleteContext(context);
            SDL_DestroyWindow(window);

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
            window = SDL_CreateWindow("hidden", 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
            context = SDL_GL_CreateContext(window);
            if (gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress) == 0)
            {
                hydra::log(
                    "Failed to load OpenGL ES functions when trying to get OpenGL ES version");
                globalHostInfo->openGlEsVersion = HC_OPENGL_ES_NOT_SUPPORTED;
            }
            else
            {
                int major, minor;
                glGetIntegerv(GL_MAJOR_VERSION, &major);
                glGetIntegerv(GL_MINOR_VERSION, &minor);
                globalHostInfo->openGlEsVersion = (HcOpenGlEsVersion)((major << 16) | minor);
                printf("OpenGL ES version: %d.%d\n", major, minor);
            }

            SDL_GL_DeleteContext(context);
            SDL_DestroyWindow(window);
        });
        context.join();

        uint32_t vkVersion;
        vkEnumerateInstanceVersion(&vkVersion);
        int major = VK_VERSION_MAJOR(vkVersion);
        int minor = VK_VERSION_MINOR(vkVersion);
        globalHostInfo->vulkanVersion = (HcVulkanVersion)((major << 16) | minor);
        printf("Vulkan version: %d.%d\n", major, minor);

#ifdef HYDRA_WINDOWS
        printf("Direct3d stuff\n");
        globalHostInfo->direct3DVersion = HC_DIRECT3D_NOT_SUPPORTED;
        // DWORD dwVersion;
        // DWORD dwRevision;
        // if (DirectXSetupGetVersion(&dwVersion, &dwRevision))
        // {
        //     printf("DirectX version is %d.%d.%d.%d\n",
        //         HIWORD(dwVersion), LOWORD(dwVersion),
        //         HIWORD(dwRevision), LOWORD(dwRevision));
        // }
#else
        // TODO: can we use dxvk? Is it too ambitious?
        globalHostInfo->direct3DVersion = HC_DIRECT3D_NOT_SUPPORTED;
#endif
        globalHostInfo->metalVersion = HC_METAL_NOT_SUPPORTED;
    }

    std::memcpy(hostInfo, globalHostInfo, sizeof(HcHostInfo));
}

HcResult hydra_hcGetInputsSync(const HcInputRequest* const* requests, int requestCount,
                               const int64_t* const* values)
{
    printf("STUB: hcGetInputsSync\n");
    return HC_SUCCESS;
}

HcResult hydra_hcReconfigureEnvironment(const HcEnvironmentInfo* environmentInfo)
{
    printf("STUB: hcReconfigureEnvironment\n");
    return HC_SUCCESS;
}

HcResult hydra_hcPushSamples(const HcAudioData* audioData)
{
    printf("STUB: hcPushSamples\n");
    return HC_SUCCESS;
}

HcResult hydra_hcSwPushVideoFrame(const HcImageData* image)
{
    printf("STUB: hcSwPushVideoFrame\n");
    return HC_SUCCESS;
}

HcResult hydra_hcGlMakeCurrent()
{
    printf("STUB: hcGlMakeCurrent\n");
    return HC_SUCCESS;
}

HcResult hydra_hcGlSwapBuffers()
{
    printf("STUB: hcGlSwapBuffers\n");
    return HC_SUCCESS;
}

void* hydra_hcGlGetProcAddress(const char* name)
{
    printf("STUB: hcGlGetProcAddress\n");
    return nullptr;
}

HcResult hydra_hcSetCallbacks(const HcCallbacks* callbacks)
{
    printf("STUB: hcSetCallbacks\n");
    return HC_SUCCESS;
}

void* hydra_GetAddress(const char* name)
{
    using namespace hydra::common;
    uint32_t hash = str_hash(name);

    switch (hash)
    {
        case str_hash("hcGetHostInfo"):
            return (void*)hydra_hcGetHostInfo;
        case str_hash("hcGetInputsSync"):
            return (void*)hydra_hcGetInputsSync;
        case str_hash("hcReconfigureEnvironment"):
            return (void*)hydra_hcReconfigureEnvironment;
        case str_hash("hcPushSamples"):
            return (void*)hydra_hcPushSamples;
        case str_hash("hcSwPushVideoFrame"):
            return (void*)hydra_hcSwPushVideoFrame;
        case str_hash("hcGlMakeCurrent"):
            return (void*)hydra_hcGlMakeCurrent;
        case str_hash("hcGlSwapBuffers"):
            return (void*)hydra_hcGlSwapBuffers;
        case str_hash("hcGlGetProcAddress"):
            return (void*)hydra_hcGlGetProcAddress;
        case str_hash("hcSetCallbacks"):
            return (void*)hydra_hcSetCallbacks;
        default:
            return nullptr;
    }
}
}