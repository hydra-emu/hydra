#include "n64_ai.hxx"
#include "n64_addresses.hxx"
#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <miniaudio.h>

namespace hydra::N64
{
    void hungry_for_more(ma_device* device, void* out, const void*, ma_uint32 frames)
    {
        auto& ai = *static_cast<Ai*>(device->pUserData);
        if (ai.ai_frequency_ == 0)
        {
            return;
        }
        // frame: 2 channels * 2 bytes per sample
        auto bytes_per_frame =
            ma_get_bytes_per_frame(device->playback.format, device->playback.channels);
        ma_resampler_config config = ma_resampler_config_init(ma_format_s16, 2,
                                                              ai.ai_frequency_, // sample rate in
                                                              HOST_SAMPLE_RATE, // sample rate out
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
            Logger::Fatal("Failed to create resampler: {}", res);
        }
        ma_uint64 frames_in = static_cast<float>(frames * ai.ai_frequency_) / HOST_SAMPLE_RATE;
        if (ai.ai_buffer_.size() * sizeof(int16_t) < frames_in * bytes_per_frame * 3)
        {
            ai.hungry_ = true;
            if (ai.ai_buffer_.size() * sizeof(int16_t) < frames_in * bytes_per_frame)
            {
                return;
            }
        }
        else
        {
            ai.hungry_ = false;
        }
        ma_uint64 frames_out = frames;
        std::vector<int16_t> buffer;
        buffer.resize(frames_out * 2);
        ma_result result = ma_resampler_process_pcm_frames(&resampler, ai.ai_buffer_.data(),
                                                           &frames_in, buffer.data(), &frames_out);
        if (result != MA_SUCCESS)
        {
            Logger::Fatal("Failed to resample: {}", result);
        }
        std::memcpy(out, buffer.data(), frames_out * 2 * 2);
        ai.ai_buffer_.erase(ai.ai_buffer_.begin(), ai.ai_buffer_.begin() + frames_in * 2);
    }

    Ai::Ai()
    {
        ma_device_config config = ma_device_config_init(ma_device_type_playback);
        config.playback.format = ma_format_s16;
        config.playback.channels = 2;
        config.sampleRate = HOST_SAMPLE_RATE;
        config.dataCallback = hungry_for_more;
        config.pUserData = this;

        if (ma_device_init(NULL, &config, &ai_device_) != MA_SUCCESS)
        {
            Logger::Fatal("Failed to open audio device");
        }
        ma_device_start(&ai_device_);
    }

    Ai::~Ai()
    {
        ma_device_uninit(&ai_device_);
    }

    void Ai::Reset()
    {
        ai_dma_count_ = 0;
    }

    void Ai::WriteWord(uint32_t addr, uint32_t data)
    {
        switch (addr)
        {
            case AI_STATUS:
            {
                mi_interrupt_->AI = false;
                break;
            }
            case AI_DRAM_ADDR:
            {
                uint32_t address = data & 0xffffff;
                if (ai_dma_count_ < 2)
                {
                    ai_dma_addresses_[ai_dma_count_] = address;
                }
                break;
            }
            case AI_LEN:
            {
                uint32_t length = (data & 0x3ffff) & ~0b111;
                if (ai_dma_count_ < 2 && length != 0)
                {
                    ai_dma_lengths_[ai_dma_count_] = length;
                    ai_dma_count_++;
                }
                break;
            }
            case AI_CONTROL:
            {
                ai_enabled_ = data & 1;
                if (ai_enabled_)
                {
                    ma_device_start(&ai_device_);
                }
                else
                {
                    ma_device_stop(&ai_device_);
                }
                break;
            }
            case AI_DACRATE:
            {
                uint32_t dac_rate = data & 0b11111111111111;
                ai_frequency_ = std::max(1u, 93'750'000 / 2 / (dac_rate + 1)) * 1.037;
                Logger::Warn("New sample rate: {}Hz", ai_frequency_);
                ai_period_ = 93'750'000 / ai_frequency_;
                break;
            }
            case AI_BITRATE:
            {
                ai_bitrate_ = data & 0xf;
                break;
            }
            default:
            {
                Logger::Warn("AI: Unhandled write to {:08x} = {:08x}", addr, data);
                break;
            }
        }
    }

    uint32_t Ai::ReadWord(uint32_t addr)
    {
        switch (addr)
        {
            case AI_STATUS:
            {
                uint32_t ret = 0;
                ret |= (ai_dma_count_ > 1) << 0;
                ret |= 1 << 20;
                ret |= 1 << 24;
                ret |= (ai_dma_count_ > 0) << 30;
                ret |= (ai_dma_count_ > 1) << 31;
                return ret;
            }
            case AI_LEN:
            {
                return ai_dma_lengths_[0];
            }
            default:
            {
                Logger::Warn("AI: Unhandled read from {:08x}", addr);
                return 0;
            }
        }
    }

    void Ai::Step()
    {
        ai_cycles_++;
        while (ai_cycles_ > ai_period_)
        {
            ai_cycles_ -= ai_period_;
            if (ai_dma_count_ == 0)
            {
                return;
            }
            uint32_t address = ai_dma_addresses_[0];
            address &= 0x7ff'ffff;
            uint32_t data = *reinterpret_cast<uint32_t*>(rdram_ptr_ + address);
            int16_t left = (static_cast<int16_t>(data >> 16));
            int16_t right = (static_cast<int16_t>(data & 0xffff));
            ai_buffer_.push_back(__builtin_bswap16(left));
            ai_buffer_.push_back(__builtin_bswap16(right));
            if (ai_buffer_.size() > 200000)
            {
                Logger::Fatal("AI buffer overflow");
            }
            ai_dma_addresses_[0] += 4;
            ai_dma_lengths_[0] -= 4;
            if (ai_dma_lengths_[0] == 0)
            {
                mi_interrupt_->AI = true;
                ai_dma_count_--;
                if (ai_dma_count_ > 0)
                {
                    ai_dma_addresses_[0] = ai_dma_addresses_[1];
                    ai_dma_lengths_[0] = ai_dma_lengths_[1];
                }
            }
        }
    }
} // namespace hydra::N64