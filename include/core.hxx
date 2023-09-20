#pragma once

#include <cstdint>
#include <future>
#include <vector>

namespace hydra
{

    enum class InputDevice
    {
        Joypad,
        Mouse,
    };

    enum InputButton
    {
        AnalogHorizontal_0,
        AnalogVertical_0,
        AnalogHorizontal_1,
        AnalogVertical_1,
        DPadUp_0,
        DPadDown_0,
        DPadLeft_0,
        DPadRight_0,
        DPadUp_1,
        DPadDown_1,
        DPadLeft_1,
        DPadRight_1,
        A,
        B,
        Z,
        X,
        Y,
        Start,
        Select,
        L1,
        R1,
        L2,
        R2,
        L3,
        R3,

        InputCount,
    };

    enum class VideoFormat
    {
        RGBA8888,
        RGBA5551,
    };

    struct VideoInfo
    {
        std::vector<uint8_t> data{};
        uint32_t width = 0, height = 0;
        VideoFormat format;
    };

    struct AudioInfo
    {
        std::vector<int16_t> data{};
    };

    struct InputInfo
    {
        uint8_t player;
        InputDevice device;
        InputButton button;
    };

    class Core
    {
    public:
        Core() = default;
        virtual ~Core() = default;
        Core(Core&&) = default;
        Core& operator=(Core&&) = default;

        virtual bool LoadFile(const std::string& type, const std::string& path) = 0;
        std::future<void> RunFrameAsync();
        virtual void Reset() = 0;
        virtual void SetVideoCallback(std::function<void(const VideoInfo&)> callback) = 0;
        virtual void SetAudioCallback(std::function<void(const AudioInfo&)> callback) = 0;
        virtual void SetPollInputCallback(std::function<void()> callback) = 0;
        virtual void SetReadInputCallback(std::function<int8_t(const InputInfo&)> callback) = 0;

    protected:
        int host_sample_rate_ = 48000;

    private:
        virtual void run_frame() = 0;

        Core(const Core&) = delete;
        Core& operator=(const Core&) = delete;
    };

} // namespace hydra