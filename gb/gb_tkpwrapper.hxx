#pragma once

#include <array>
#include <emulator.hxx>
#include <gb/gb_addresses.hxx>
#include <gb/gb_apu.hxx>
#include <gb/gb_apu_ch.hxx>
#include <gb/gb_breakpoint.hxx>
#include <gb/gb_bus.hxx>
#include <gb/gb_cpu.hxx>
#include <gb/gb_ppu.hxx>
#include <gb/gb_timer.hxx>

namespace hydra
{
    namespace Applications
    {
        class GameboyRomData;
    }

    namespace Gameboy::QA
    {
        class TestGameboy;
    }
} // namespace hydra
class MmioViewer;

namespace hydra::Gameboy
{
    class Gameboy_TKPWrapper : public Emulator
    {
        TKP_EMULATOR(Gameboy_TKPWrapper);

    private:
        using GameboyPalettes = std::array<std::array<float, 3>, 4>;
        using GameboyKeys = std::array<uint32_t, 4>;
        using CPU = hydra::Gameboy::CPU;
        using PPU = hydra::Gameboy::PPU;
        using APU = hydra::Gameboy::APU;
        using ChannelArrayPtr = hydra::Gameboy::ChannelArrayPtr;
        using ChannelArray = hydra::Gameboy::ChannelArray;
        using Bus = hydra::Gameboy::Bus;
        using Timer = hydra::Gameboy::Timer;
        using Cartridge = hydra::Gameboy::Cartridge;
        using GameboyBreakpoint = hydra::Gameboy::Utils::GameboyBreakpoint;

    public:
        // Used by automated tests
        void Update()
        {
            update();
        }

    private:
        ChannelArrayPtr channel_array_ptr_;
        Bus bus_;
        APU apu_;
        PPU ppu_;
        Timer timer_;
        CPU cpu_;
        GameboyKeys direction_keys_;
        GameboyKeys action_keys_;
        uint8_t &joypad_, &interrupt_flag_;
        inline void update_audio_sync();
        friend class hydra::Gameboy::QA::TestGameboy;
        friend class ::MmioViewer;
    };
} // namespace hydra::Gameboy
