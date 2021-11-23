#ifndef TKP_BASE_DISASSEMBLER_H
#define TKP_BASE_DISASSEMBLER_H
#include "base_application.h"
#include "emulator.h"
namespace TKPEmu::Applications {
	struct BaseDisassembler : public IMApplication {
    protected:
        Emulator* emulator_;
        bool* rom_loaded_ = nullptr;
        virtual void v_draw() = 0;
    public:
        BaseDisassembler(bool* rom_loaded) : rom_loaded_(rom_loaded) {};
        virtual ~BaseDisassembler() = default;
        bool OpenGotoPopup = false;
        void Draw(const char* title, bool* p_open = NULL) override {
            TKPEmu::Applications::IMApplication::SetupWindow();
            if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_MenuBar)) {
                ImGui::End();
                return;
            }
            v_draw();
            ImGui::End();
        };
        void SetEmulator(Emulator* emulator) {
            emulator_ = emulator;
        }
	};
}
#endif
