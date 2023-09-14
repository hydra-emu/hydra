#pragma once

#include <core.hxx>
#include <n64/core/n64_impl.hxx>

namespace hydra
{
    class HydraCore_N64 : public Core
    {
    public:
        HydraCore_N64();
        ~HydraCore_N64();

        bool LoadFile(const std::string& type, const std::string& path) override;
        void Reset() override;
        void SetVideoCallback(std::function<void(const VideoInfo&)> callback) override;
        void SetAudioCallback(std::function<void(const AudioInfo&)> callback) override;
        void SetPollInputCallback(std::function<void()> callback) override;
        void SetReadInputCallback(std::function<int8_t(const InputInfo&)> callback) override;

    private:
        void run_frame() override;
        void audio_callback_wrapper(const std::vector<int16_t>& in, int frequency_in);
        int16_t read_input_callback_wrapper(int player, int device, int button);

        bool ipl_loaded_ = false;
        N64::N64 impl_;

        std::function<void(const VideoInfo&)> video_callback_;
        std::function<void(const AudioInfo&)> audio_callback_;
        std::function<int8_t(const InputInfo&)> read_input_callback_;
    };
} // namespace hydra