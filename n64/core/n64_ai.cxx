#include <algorithm>
#include <compatibility.hxx>
#include <fmt/format.h>
#include <fstream>
#include <miniaudio.h>
#include <n64/core/n64_addresses.hxx>
#include <n64/core/n64_ai.hxx>

namespace hydra::N64
{
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
                // if (ai_enabled_)
                // {
                //     ma_device_start(&ai_device_);
                // }
                // else
                // {
                //     ma_device_stop(&ai_device_);
                // }
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
            ai_buffer_.push_back(bswap16(left));
            ai_buffer_.push_back(bswap16(right));
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