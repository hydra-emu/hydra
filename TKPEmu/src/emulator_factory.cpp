#include <iostream>
#include <lib/str_hash.h>
#include <include/emulator_factory.h>
#include <include/generic_tool_make.h>
#include <GameboyTKP/gb_disassembler.h>
#include <GameboyTKP/gb_tracelogger.h>
#include <GameboyTKP/gb_romdata.h>
#include <N64TKP/n64_tkpromdisassembly.hxx>

namespace TKPEmu {
    void EmulatorFactory::LoadEmulatorTools(std::vector<std::unique_ptr<IMApplication>>& tools, std::shared_ptr<Emulator> emulator, EmuType emu_type) {
        switch (emu_type) {
            case EmuType::Gameboy: {
                tools.push_back(std::make_unique<Applications::GameboyRomData>("Rom data", "Gameboy rom data"));
                tools.push_back(std::make_unique<Applications::GameboyDisassembler>("Disassembler", "Gameboy disassembler"));
                tools.push_back(std::make_unique<Applications::GameboyTracelogger>("Tracelogger", "Gameboy tracelogger"));
                break;
            }
            case EmuType::N64: {
                tools.push_back(std::make_unique<Applications::N64_RomDisassembly>("Rom disassembly", "N64 rom disassembly"));
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
    void EmulatorFactory::LoadGenericTools(std::vector<std::unique_ptr<TKPEmu::Applications::Drawable>>& tools, std::shared_ptr<Emulator> emulator, EmuType emu_type) {
        tools.push_back(MakeGenericTool<TKPEmu::Applications::Type::GenericDisassembler>(emu_type, emulator));
    }
    // TODO: detect emutype by magic bytes instead of extension
    EmuType EmulatorFactory::GetEmulatorType(std::filesystem::path path) {
        auto ext = str_hash(path.extension().c_str());
        switch (ext) {
            case str_hash(".gbc"):
            case str_hash(".gb"): {
                return EmuType::Gameboy;
            }
            case str_hash(".n64"):
            case str_hash(".N64"):
            case str_hash(".z64"): {
                return EmuType::N64;
            }
            case str_hash(".ch8"): {
                return EmuType::Chip8;
            }
        }
        return EmuType::None;
    }
}