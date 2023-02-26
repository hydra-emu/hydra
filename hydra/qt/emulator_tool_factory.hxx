#ifndef TKP_EMULATOR_TOOL_FACTORY_H
#define TKP_EMULATOR_TOOL_FACTORY_H
#include <memory>
#include <qt/n64_debugger.hxx>
#include <lib/messagequeue.hxx>
#include <include/emulator_types.hxx>

struct EmulatorToolFactory {
    static QWidget* CreateDebugger(
        bool& open,
        std::shared_ptr<TKPEmu::Tools::MQBase> mq,
        TKPEmu::EmuType emu_type,
        QWidget* parent,
        TKPEmu::Emulator* emulator
    )
    {
        switch (emu_type) {
            case TKPEmu::EmuType::N64: {
                auto ret = new N64Debugger(open, mq, parent);
                ret->SetEmulator(dynamic_cast<TKPEmu::N64::N64_TKPWrapper*>(emulator));
                return ret;
            }
        }
        return nullptr;
    }
};
#endif