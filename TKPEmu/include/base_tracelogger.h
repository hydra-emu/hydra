#ifndef TKP_BASE_TRACELOGGER_H
#define TKP_BASE_TRACELOGGER_H
#include "base_application.h"
namespace TKPEmu::Applications {
    struct BaseTracelogger : public IMApplication {
    protected:
        Emulator* emulator_ = nullptr;
        virtual void v_draw() = 0;
    public:
        BaseTracelogger() {};
        virtual ~BaseTracelogger() = default;
        void SetEmulator(Emulator* emulator) {
            emulator_ = emulator;
        }
        virtual void SetFile(std::string file) = 0;
        void Draw(const char* title, bool* p_open = nullptr) final {
            TKPEmu::Applications::IMApplication::SetupWindow();
            if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_MenuBar)) {
                ImGui::End();
                return;
            }
            v_draw();
            ImGui::End();
        }
    };
}
#endif
