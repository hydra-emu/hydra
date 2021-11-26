#include <iostream>
#include "../include/emulator_factory.h"
#include "../gb_tkp/gb_disassembler.h"
#include "../gb_tkp/gb_tracelogger.h"
namespace TKPEmu {
    void EmulatorFactory::LoadEmulatorTools(std::vector<std::unique_ptr<IMApplication>>& tools, EmuType emu_type) {
        tools.clear();
        switch (emu_type) {
            case EmuType::Gameboy: {
                tools.push_back(std::make_unique<Applications::GameboyDisassembler>());
                tools.push_back(std::make_unique<Applications::GameboyTracelogger>());
                break;
            }
            default: {
                std::cerr << "LoadEmulatorTools was not implemented for this emulator\nNo tools loaded..." << std::endl;
                break;
            }
        }
    }
}