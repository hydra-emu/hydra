#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <log.hxx>
#include <n64/core/n64_types.hxx>
#include <vector>

namespace hydra::N64
{
    class N64;
    class RCP;
    class CPU;
    class CPUBus;

    constexpr uint32_t HOST_SAMPLE_RATE = 48000;

    class Ai
    {
    public:
        void Reset();
        void InstallBuses(uint8_t* rdram_ptr);
        void SetInterruptCallback(std::function<void(bool)> callback);
        void SetAudioCallback(
            std::function<void(const std::vector<int16_t>& in, int frequency_in)> callback);
        void Step();
        uint32_t ReadWord(uint32_t addr);
        void WriteWord(uint32_t addr, uint32_t data);

    private:
        uint32_t ai_control_ = 0;
        uint32_t ai_bitrate_ = 0;
        uint32_t ai_frequency_ = 0;
        uint32_t ai_period_ = 93750000 / 48000;
        bool ai_enabled_ = false;
        uint8_t ai_dma_count_ = 0;
        uint32_t ai_cycles_ = 0;
        std::function<void(bool)> interrupt_callback_;
        std::function<void(const std::vector<int16_t>&, int)> audio_callback_;

        std::array<uint32_t, 2> ai_dma_addresses_{};
        std::array<uint32_t, 2> ai_dma_lengths_{};

        uint8_t* rdram_ptr_ = nullptr;

        std::vector<int16_t> ai_buffer_{};

        friend class hydra::N64::N64;
        friend class hydra::N64::RCP;
        friend class hydra::N64::CPU;
        friend class hydra::N64::CPUBus;
    };
} // namespace hydra::N64