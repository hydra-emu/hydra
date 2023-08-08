#pragma once

#include <gb/gb_apu_ch.hxx>
#include <queue>

namespace hydra::Gameboy
{
    // This class is solely for sound output and is not needed to pass sound
    // emulation tests.
    // All computation for this class happens in gb_bus and gb_apu_ch
    class APU
    {
    public:
        APU(ChannelArrayPtr channel_array_ptr, uint8_t& NR52);
        ~APU();
        void InitSound();
        void Update(int clk);

        inline bool IsQueueEmpty()
        {
            return samples_.empty();
        }

        bool UseSound = false;

    private:
        // QAudioSink* audio_sink_;
        std::array<int16_t, 512> samples_;
        size_t sample_index_ = 0;
        uint8_t& NR52_;
        ChannelArrayPtr channel_array_ptr_;
        bool init_ = false;
    };
} // namespace hydra::Gameboy
