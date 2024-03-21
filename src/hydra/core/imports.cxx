// Define the hydra core API imports

#include "compatibility.hxx"
#include <cstdio>
#include <hydra/core.h>

extern "C" {

void hydra_hcGetHostInfo(HcHostInfo* hostInfo)
{
    printf("STUB: hcGetHostInfo\n");
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
    uint32_t hash = hydra::str_hash(name);

    switch (hash)
    {
        case hydra::str_hash("hcGetHostInfo"):
            return (void*)hydra_hcGetHostInfo;
        case hydra::str_hash("hcGetInputsSync"):
            return (void*)hydra_hcGetInputsSync;
        case hydra::str_hash("hcReconfigureEnvironment"):
            return (void*)hydra_hcReconfigureEnvironment;
        case hydra::str_hash("hcPushSamples"):
            return (void*)hydra_hcPushSamples;
        case hydra::str_hash("hcSwPushVideoFrame"):
            return (void*)hydra_hcSwPushVideoFrame;
        case hydra::str_hash("hcGlMakeCurrent"):
            return (void*)hydra_hcGlMakeCurrent;
        case hydra::str_hash("hcGlSwapBuffers"):
            return (void*)hydra_hcGlSwapBuffers;
        case hydra::str_hash("hcGlGetProcAddress"):
            return (void*)hydra_hcGlGetProcAddress;
        case hydra::str_hash("hcSetCallbacks"):
            return (void*)hydra_hcSetCallbacks;
        default:
            return nullptr;
    }
}
}