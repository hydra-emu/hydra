#ifndef GB_WIIWRAPPER_H
#define GB_WIIWRAPPER_H
#include <gb/gb_addresses.h>
#include <gb/gb_cpu.h>
#include <gb/gb_ppu.h>
#include <gb/gb_bus.h>
#include <gb/gb_timer.h>
#include <gb/gb_apu.h>
#include <gb/gb_apu_ch.h>

class GB_WiiWrapper {
public:
    GB_WiiWrapper();
    void Update();
    void* GetScreenData();
    void LoadCartridge(void* data);
    void HandleKeyDown(uint32_t key);
    void HandleKeyUp(uint32_t key);
    using CPU = hydra::Gameboy::CPU;
    using GameboyPalettes = std::array<std::array<float, 3>,4>;
    using GameboyKeys = std::array<uint32_t, 4>;
    using PPU = hydra::Gameboy::PPU;
    using APU = hydra::Gameboy::APU;
    using ChannelArrayPtr = hydra::Gameboy::ChannelArrayPtr;
    using ChannelArray = hydra::Gameboy::ChannelArray;
    using Bus = hydra::Gameboy::Bus;
    using Timer = hydra::Gameboy::Timer;
    using Cartridge = hydra::Gameboy::Cartridge;
    ChannelArrayPtr channel_array_ptr_;
    CPU cpu_;
    Bus bus_;
    APU apu_;
    PPU ppu_;
    Timer timer_;
    uint8_t& joypad_, &interrupt_flag_;
};
#endif