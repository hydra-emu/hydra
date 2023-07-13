#pragma once
#ifndef TKP_GB_APU_CH_H
#define TKP_GB_APU_CH_H
#include <array>
#include <memory>
namespace hydra::Gameboy {
    constexpr int Waveforms[4] = { 0b00000001, 0b00000011, 0b00001111, 0b11111100 };
    struct APUChannel {
        // TODO: some of these variables dont need to be public
        int FrequencyTimer = 0;
        int WaveDutyPattern = 2;
        int WaveDutyPosition = 0;
        int MagicDivider = 2;
        int FrameSequencer = 0; // TODO: unimplemented
        bool LengthCtrEnabled = false;
        bool VolEnvEnabled = true;
        uint8_t EnvelopeCurrentVolume = 0xF;
        bool EnvelopeIncrease = false;
        int EnvelopePeriod = 0;
        int SweepPeriod = 0;
        bool SweepIncrease = false;
        int SweepShift = 0;
        bool SweepEnabled = false;
        bool HasSweep = false;
        int SweepTimer = 0;
        int ShadowFrequency = 0;
        int Frequency = 0;
        int LengthTimer = 0;
        int LengthHalf = 0;
        int LengthData = 0;
        bool LengthDecOne = false;
        int LengthInit = 64;
        int PeriodTimer = 0;
        int DACInput = 0;
        float DACOutput = 1;
        bool DACEnabled = true;
        bool LeftEnabled = false;
        uint8_t LeftVolume = 0;
        bool RightEnabled = false;
        uint8_t RightVolume = 0;
        bool DisableChannelFlag = false;
        unsigned Divisor = 0;
        bool WidthMode = false;
        unsigned DivisorShift = 0;
        uint16_t LFSR = 0;

        void StepWaveGeneration(int cycles);
        void StepWaveGenerationCh4(int cycles);
        void StepFrameSequencer();
        bool GetAmplitude();
        void ClockLengthCtr();
        void ClockVolEnv();
        void ClockSweep();
        void CalculateSweepFreq();
        inline uint8_t GlobalVolume() {
            return ((LeftEnabled * LeftVolume) || (RightEnabled * RightVolume));
        }
    private:
        int new_frequency = 0;
    };
    using ChannelArray = std::array<APUChannel, 4>;
    using ChannelArrayPtr = std::shared_ptr<ChannelArray>;
}
#endif
