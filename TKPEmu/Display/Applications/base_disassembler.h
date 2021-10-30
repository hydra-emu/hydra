#ifndef TKP_BASE_DISASSEMBLER_H
#define TKP_BASE_DISASSEMBLER_H
#include "base_application.h"
namespace TKPEmu::Applications {
	struct BaseDisassembler : public IMApplication {
    public:
        std::atomic_bool Loaded = false;
        BaseDisassembler(bool* rom_loaded) : IMApplication(rom_loaded) {};
        virtual void SetEmulator(Emulator* emulator) = 0;
        static void DrawMenuEmulation(Emulator* emulator, bool* rom_loaded) {
            if (!*rom_loaded) {
                ImGui::MenuItem("Pause", NULL, false, *rom_loaded);
            }
            else {
                if (ImGui::MenuItem("Pause", NULL, emulator->Paused.load(), *rom_loaded)) {
                    emulator->Paused.store(!emulator->Paused.load());
                    emulator->Step.store(true);
                    emulator->Step.notify_all();
                }
            }
            if (ImGui::MenuItem("Stop", 0, false, *rom_loaded)) {
                *rom_loaded = false;
                ResetEmulatorState(emulator);
            }
            ImGui::EndMenu();
        }
        static void ResetEmulatorState(Emulator* emulator) {
            emulator->Step.store(true);
            emulator->Paused.store(false);
            emulator->Stopped.store(true);
            emulator->Step.notify_all();
        }
	};
}
#endif