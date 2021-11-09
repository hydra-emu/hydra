#ifndef TKP_BASE_DISASSEMBLER_H
#define TKP_BASE_DISASSEMBLER_H
#include "base_application.h"
namespace TKPEmu::Applications {
	struct BaseDisassembler : public IMApplication {
    protected:
        bool* rom_loaded_ = nullptr;
        BaseDisassembler() = delete;
        virtual void v_draw() = 0;
    public:
        BaseDisassembler(bool* rom_loaded) : rom_loaded_(rom_loaded) {};
        virtual ~BaseDisassembler() = default;
        void Draw(const char* title, bool* p_open = NULL) final {
            ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
            if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_MenuBar)) {
                ImGui::End();
                return;
            }
            v_draw();
            ImGui::End();
        };
        static void DrawMenuEmulation(Emulator* emulator, bool* rom_loaded) {
            if (!*rom_loaded) {
                ImGui::MenuItem("Pause", "Ctrl+P", false, *rom_loaded);
            }
            else {
                if (ImGui::MenuItem("Pause", "Ctrl+P", emulator->Paused.load(), *rom_loaded)) {
                    emulator->Paused.store(!emulator->Paused.load());
                    emulator->Step.store(true);
                    emulator->Step.notify_all();
                }
            }
            if (ImGui::MenuItem("Reset", "Ctrl+R", false, *rom_loaded)) {
                // Sets the stopped flag on the thread to true and then waits for it to become false
                // The thread sets the flag to false upon exiting
                ResetEmulatorState(emulator);
                emulator->StartDebug();
            }
            if (ImGui::MenuItem("Stop", NULL, false, *rom_loaded)) {
                *rom_loaded = false;
                ResetEmulatorState(emulator);
            }
            ImGui::EndMenu();
        }
        static void ResetEmulatorState(Emulator* emulator) {
            emulator->Step.store(true);
            emulator->Paused.store(false);
            emulator->Stopped.store(true);
            emulator->Step.notify_one();
        }
	};
}
#endif