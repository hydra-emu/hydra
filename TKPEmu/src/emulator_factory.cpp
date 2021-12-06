#include <iostream>
#include "../lib/str_hash.h"
#include "../include/emulator_factory.h"
#include "../gb_tkp/gb_disassembler.h"
#include "../gb_tkp/gb_tracelogger.h"
namespace TKPEmu {
    void EmulatorFactory::LoadEmulatorTools(std::vector<std::unique_ptr<IMApplication>>& tools, Emulator* emulator, EmuType emu_type) {
        switch (emu_type) {
            case EmuType::Gameboy: {
                tools.push_back(std::make_unique<Applications::GameboyDisassembler>("Disassembler", "Gameboy disassembler"));
                std::cout << "Disassembler loaded" << std::endl;
                tools.push_back(std::make_unique<Applications::GameboyTracelogger>("Tracelogger", "Gameboy tracelogger"));
                std::cout << "Tracelogger loaded" << std::endl;
                break;
            }
            default: {
                std::cerr << "LoadEmulatorTools was not implemented for this emulator\nNo tools loaded..." << std::endl;
                break;
            }
        }
        for (const auto& app : tools) {
            app->SetEmulator(emulator);
        }
    }
    EmuType EmulatorFactory::GetEmulatorType(std::filesystem::path path) {
        auto ext = str_hash(path.extension().c_str());
        switch (ext) {
            case str_hash(".gb"): {
                return EmuType::Gameboy;
            }
        }
        return EmuType::None;
    }
}