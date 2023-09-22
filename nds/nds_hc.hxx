#pragma once

#include <core.hxx>

namespace hydra
{
    class HydraCore_NDS : public Core
    {
    public:
        HydraCore_NDS() = default;
        ~HydraCore_NDS() = default;

        bool LoadFile(const std::string& type, const std::string& path) override;
        void Reset() override;
        void SetVideoCallback(std::function<void(const VideoInfo&)> callback) override;
        void SetAudioCallback(std::function<void(const AudioInfo&)> callback) override;
        void SetPollInputCallback(std::function<void()> callback) override;
        void SetReadInputCallback(std::function<int8_t(const InputInfo&)> callback) override;

    private:
        void run_frame() override;
    };
} // namespace hydra