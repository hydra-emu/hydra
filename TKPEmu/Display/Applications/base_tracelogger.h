#ifndef TKP_BASE_TRACELOGGER_H
#define TKP_BASE_TRACELOGGER_H
#include "base_application.h"
namespace TKPEmu::Applications {
    struct BaseTracelogger : public IMApplication {
    private:
        BaseTracelogger() = delete;
    protected:
        Emulator* emulator_ = nullptr;
        bool* log_mode_ = nullptr;
        virtual void v_draw() = 0;
    public:
        BaseTracelogger(bool* log_mode) : log_mode_(log_mode) {};
        virtual ~BaseTracelogger() = default;
        void SetEmulator(Emulator* emulator) {
            emulator_ = emulator;
        }
        virtual void SetFile(std::string file) = 0;
        void Draw(const char* title, bool* p_open = nullptr) final {
            ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
            if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_MenuBar)) {
                ImGui::End();
                return;
            }
            if (!*log_mode_) {
                ImGui::Text("You need to activate LogMode to use the tracelogger.");
                if (ImGui::Button("Restart in LogMode")) {
                    
                }
            }
            else {
                v_draw();
            }
            ImGui::End();
        }
    };
}
#endif