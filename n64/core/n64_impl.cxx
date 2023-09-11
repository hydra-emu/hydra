#include <chrono>
#include <iostream>
#include <n64/core/n64_impl.hxx>

#define PROFILING
#ifdef PROFILING
#include <valgrind/callgrind.h>
#else
#define CALLGRIND_START_INSTRUMENTATION
#define CALLGRIND_STOP_INSTRUMENTATION
#endif

namespace hydra::N64
{
    N64::N64() : cpubus_(rcp_), cpu_(cpubus_, rcp_)
    {
        Reset();
    }

    bool N64::LoadCartridge(std::string path)
    {
        return cpu_.cpubus_.LoadCartridge(path);
    }

    bool N64::LoadIPL(std::string path)
    {
        if (!path.empty())
        {
            return cpu_.cpubus_.LoadIPL(path);
        }
        return false;
    }

    void N64::RunFrame()
    {
        CALLGRIND_START_INSTRUMENTATION;
        static int cycles = 0;
        for (int f = 0; f < 1; f++)
        { // fields
            for (int hl = 0; hl < cpu_.rcp_.vi_.num_halflines_; hl++)
            { // halflines
                cpu_.rcp_.vi_.vi_v_current_ = (hl << 1) + f;
                cpu_.check_vi_interrupt();
                while (cycles <= cpu_.rcp_.vi_.cycles_per_halfline_)
                {
                    static int cpu_cycles = 0;
                    cpu_cycles++;
                    cpu_.Tick();
                    rcp_.ai_.Step();
                    if (!cpu_.rcp_.rsp_.IsHalted())
                    {
                        while (cpu_cycles > 2)
                        {
                            cpu_.rcp_.rsp_.Tick();
                            if (!cpu_.rcp_.rsp_.IsHalted())
                            {
                                cpu_.rcp_.rsp_.Tick();
                            }
                            cpu_cycles -= 3;
                        }
                    }
                    else
                    {
                        cpu_cycles = 0;
                    }
                    cycles++;
                }
                cycles -= cpu_.rcp_.vi_.cycles_per_halfline_;
            }
            cpu_.check_vi_interrupt();
        }
        CALLGRIND_STOP_INSTRUMENTATION;
    }

    void N64::Reset()
    {
        cpu_.Reset();
        rcp_.Reset();
    }

    void N64::SetMousePos(int32_t x, int32_t y)
    {
        cpu_.mouse_delta_x_ = x - cpu_.mouse_x_;
        cpu_.mouse_delta_y_ = y - cpu_.mouse_y_;
        cpu_.mouse_x_ = x;
        cpu_.mouse_y_ = y;
    }

    void N64::RenderAudio(std::vector<int16_t>& data)
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
        ma_result result = ma_resampler_process_pcm_frames(&resampler, rcp_.ai_.ai_buffer_.data(),
                                                           &frames_in, data.data(), &frames_out);
        if (result != MA_SUCCESS)
        {
            Logger::Fatal("Failed to resample: {}", static_cast<int>(result));
        }
        rcp_.ai_.ai_buffer_.erase(rcp_.ai_.ai_buffer_.begin(),
                                  rcp_.ai_.ai_buffer_.begin() + frames_in * 2);
    }
} // namespace hydra::N64