#ifndef TKP_BASE_TRACELOGGER_H
#define TKP_BASE_TRACELOGGER_H
#include <filesystem>
#include "base_application.h"
#include "../lib/imgui_stdlib.h"
namespace TKPEmu::Applications {
    struct BaseTracelogger : public IMApplication {
    public:
        BaseTracelogger() {};
        virtual ~BaseTracelogger() = default;
        void SetEmulator(Emulator* emulator);
        void Draw(const char* title, bool* p_open = nullptr);
    protected:
        Emulator* emulator_ = nullptr;
    private:
        virtual void v_draw() = 0;
    };
}
#endif
