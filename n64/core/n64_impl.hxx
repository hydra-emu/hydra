#pragma once

#include <miniaudio.h>
#include <n64/core/n64_cpu.hxx>
#include <n64/core/n64_rcp.hxx>
#include <string>

class MmioViewer;

namespace hydra::N64
{
    class N64
    {
    public:
        N64();
        bool LoadCartridge(std::string path);
        bool LoadIPL(std::string path);
        void RunFrame();
        void Reset();
        void SetMousePos(int32_t x, int32_t y);

        int GetWidth()
        {
            return rcp_.vi_.width_;
        }

        int GetHeight()
        {
            return rcp_.vi_.height_;
        }

        void SetKeyState(uint32_t key, bool state)
        {
            cpu_.key_state_[key] = state;
        }

        void RenderVideo(std::vector<uint8_t>& data)
        {
            rcp_.vi_.Redraw(data);
        }

        void RenderAudio(std::vector<int16_t>& data)
        {
            if (rcp_.ai_.ai_frequency_ == 0)
            {
                return;
            }
            ma_resampler_config config =
                ma_resampler_config_init(ma_format_s16, 2,
                                         rcp_.ai_.ai_frequency_, // sample rate in
                                         HOST_SAMPLE_RATE,       // sample rate out
                                         ma_resample_algorithm_linear);
            ma_resampler resampler;

            struct Deleter
            {
                Deleter(ma_resampler& resampler) : resampler_(resampler){};

                ~Deleter()
                {
                    ma_resampler_uninit(&resampler_, nullptr);
                };

            private:
                ma_resampler& resampler_;
            } deleter(resampler);

            if (ma_result res = ma_resampler_init(&config, nullptr, &resampler))
            {
                Logger::Fatal("Failed to create resampler: {}", static_cast<int>(res));
            }

            ma_uint64 frames_in = rcp_.ai_.ai_frequency_ / 60;
            frames_in = std::min(frames_in, (ma_uint64)rcp_.ai_.ai_buffer_.size() >> 1);
            ma_uint64 frames_out = 48000 / 60;
            data.resize(frames_out * 2);
            ma_result result = ma_resampler_process_pcm_frames(
                &resampler, rcp_.ai_.ai_buffer_.data(), &frames_in, data.data(), &frames_out);
            if (result != MA_SUCCESS)
            {
                Logger::Fatal("Failed to resample: {}", static_cast<int>(result));
            }
            rcp_.ai_.ai_buffer_.erase(rcp_.ai_.ai_buffer_.begin(),
                                      rcp_.ai_.ai_buffer_.begin() + frames_in * 2);
        }

    private:
        RCP rcp_;
        CPUBus cpubus_;
        CPU cpu_;
        friend class ::MmioViewer;
    };
} // namespace hydra::N64
