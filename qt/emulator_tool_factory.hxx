#ifndef TKP_EMULATOR_TOOL_FACTORY_H
#define TKP_EMULATOR_TOOL_FACTORY_H
#include <memory>
#include <qt/n64_debugger.hxx>
#include <include/emulator_types.hxx>

struct EmulatorToolFactory {
    static QWidget* CreateDebugger(
        bool& open,
        hydra::EmuType emu_type,
        QWidget* parent,
        hydra::Emulator* emulator
    )
    {
        switch (emu_type) {
            case hydra::EmuType::N64: {
                auto ret = new N64Debugger(open, parent);
                ret->SetEmulator(dynamic_cast<hydra::N64::N64_TKPWrapper*>(emulator));
                return ret;
            }
        }
        return nullptr;
    }
};
#endif