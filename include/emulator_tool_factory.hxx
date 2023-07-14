#pragma once

#include <emulator_types.hxx>
#include <memory>
#include <qt/n64_debugger.hxx>

struct EmulatorToolFactory
{
    static QWidget* CreateDebugger(bool& open, hydra::EmuType emu_type, QWidget* parent,
                                   hydra::Emulator* emulator)
    {
        switch (emu_type)
        {
            case hydra::EmuType::N64:
            {
                auto ret = new N64Debugger(open, parent);
                ret->SetEmulator(dynamic_cast<hydra::N64::N64_TKPWrapper*>(emulator));
                return ret;
            }
        }
        return nullptr;
    }
};
