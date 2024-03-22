#pragma once

#include <filesystem>
#include <hydra/core.h>
#include <hydra/dynlib/dynlib.hxx>

namespace hydra::core
{
    struct Wrapper final
    {
    private:
        using hcGetCoreInfoPtr = void (*)(HcCoreInfo* coreInfo);
        using hcCreatePtr = HcResult (*)(HcEnvironmentInfo* environmentInfo);
        using hcDestroyPtr = HcResult (*)(const HcDestroyInfo* destroyInfo);
        using hcResetPtr = HcResult (*)(const HcResetInfo* resetInfo);
        using hcSetRunStatePtr = HcResult (*)(const HcRunStateInfo* runInfo);
        using hcLoadContentPtr = HcResult (*)(const HcContentLoadInfo* info);
        using hcGetErrorPtr = const char* (*)();

    public:
        Wrapper(const std::filesystem::path& path);
        ~Wrapper();

        bool okay() const;

        hcGetCoreInfoPtr hcGetCoreInfo;
        hcCreatePtr hcCreate;
        hcDestroyPtr hcDestroy;
        hcResetPtr hcReset;
        hcSetRunStatePtr hcSetRunState;
        hcLoadContentPtr hcLoadContent;
        hcGetErrorPtr hcGetError;

    private:
        hydra::dynlib::handle_t handle;
    };
} // namespace hydra::core