#include <hydra/common/log.hxx>
#include <hydra/common/settings.hxx>
#include <hydra/core.h>
#include <hydra/core/thread.hxx>
#include <hydra/core/wrapper.hxx>
#include <memory>
#include <thread>

std::shared_ptr<hydra::core::Wrapper> globalWrapper;
std::unique_ptr<std::thread> emulatorThread;
std::atomic_bool scheduledReset = false;
HcDriveMode driveMode = HC_DRIVE_MODE_NULL;

namespace
{
    inline static void handleError(HcResult result, std::shared_ptr<hydra::core::Wrapper> wrapper,
                                   const char* hint)
    {
        if (result != HC_SUCCESS)
        {
            switch (result)
            {
                case HC_ERROR_CORE:
                {
                    const char* error = wrapper->hcGetError();
                    if (error)
                    {
                        hydra::panic("TODO: don't panic, Core error: {}\n{}", error, hint);
                    }
                    else
                    {
                        hydra::panic(
                            "TODO: don't panic, Core error, but hcGetError returned nullptr.\n{}",
                            hint);
                    }
                    break;
                }
                default:
                    hydra::panic("TODO: don't panic, Unhandled error code: {}\n", (int)result,
                                 hint);
                    break;
            }
        }
    }

    static void entrypointSelfDriven(std::shared_ptr<hydra::core::Wrapper> wrapper)
    {
        printf("TODO: implement entrypointSelfDriven\n");
    }

    static void entrypointFrontendDriven(std::shared_ptr<hydra::core::Wrapper> wrapper)
    {
        printf("TODO: implement entrypointFrontendDriven\n");

        printf("Handle scheduled reset\n");
    }
} // namespace

namespace hydra::core::thread
{
    void start(std::shared_ptr<Wrapper> wrapper)
    {
        globalWrapper = wrapper;
        emulatorThread = std::make_unique<std::thread>([wrapper] {
            HcVideoInfo videoInfo{};
            HcAudioInfo audioInfo{};
            HcEnvironmentInfo environmentInfo{};
            environmentInfo.video = &videoInfo;
            environmentInfo.audio = &audioInfo;
            HcResult result = wrapper->hcCreate(&environmentInfo);
            handleError(result, wrapper, "Failed to create core");

            HcRunStateInfo runStateInfo{};
            runStateInfo.runState =
                settings::Config::get().startPaused ? HC_RUN_STATE_PAUSED : HC_RUN_STATE_RUNNING;
            result = wrapper->hcSetRunState(&runStateInfo);
            handleError(result, wrapper, "Failed to set run state");

            HcDriveMode mode = environmentInfo.driveMode;
            driveMode = mode;
            switch (mode)
            {
                case HC_DRIVE_MODE_SELF_DRIVEN_EXCEPT_AUDIO:
                    printf("TODO: initialize audio:\n");
                    [[fallthrough]];
                case HC_DRIVE_MODE_SELF_DRIVEN:
                    entrypointSelfDriven(wrapper);
                    break;
                case HC_DRIVE_MODE_FRONTEND_DRIVEN:
                    entrypointFrontendDriven(wrapper);
                    break;
                default:
                    hydra::panic("TODO: don't panic, Unhandled drive mode: {}\n", (int)mode);
                    break;
            }
        });
    }

    void stop()
    {
        HcRunStateInfo runStateInfo{};
        runStateInfo.runState = HC_RUN_STATE_QUIT;
        HcResult result = globalWrapper->hcSetRunState(&runStateInfo);
        handleError(result, globalWrapper, "Failed to set run state to quit");

        HcDestroyInfo destroyInfo{};
        result = globalWrapper->hcDestroy(&destroyInfo);
        handleError(result, globalWrapper, "Failed to destroy core");

        emulatorThread->join();

        if (globalWrapper.use_count() != 1)
        {
            hydra::log("Wrapper use count is not 1 while exiting core");
        }

        globalWrapper.reset();
        emulatorThread.reset();
        driveMode = HC_DRIVE_MODE_NULL;
    }

    void reset(HcResetType type)
    {
        if (!globalWrapper)
        {
            hydra::panic("Wrapper is not initialized while trying to reset core");
        }

        if (driveMode == HC_DRIVE_MODE_FRONTEND_DRIVEN)
        {
            scheduledReset = true;
        }
        else
        {
            HcResetInfo resetInfo{};
            resetInfo.resetType = type;
            HcResult result = globalWrapper->hcReset(&resetInfo);
            handleError(result, globalWrapper, "Failed to reset core");
        }
    }

    bool running()
    {
        return emulatorThread && emulatorThread->joinable();
    }
} // namespace hydra::core::thread