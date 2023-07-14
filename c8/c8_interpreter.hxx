#ifndef TKP_c8_INTER
#define TKP_c8_INTER
#include <array>
#include <cstdint>
#include <string>
#include <chrono>

namespace hydra::c8 {
    class Chip8_TKPWrapper;
    union Opcode {
        uint16_t full;
        struct {
            uint16_t _4 : 4;
            uint16_t _3 : 4;
            uint16_t _2 : 4;
            uint16_t _1 : 4;
        };
        struct {
            uint16_t _2b : 8;
            uint16_t _1b : 8;
        };
    };
    class Interpreter {
    public:
        Interpreter();
        void Update();
        uint8_t* GetScreenData() {
            return &screen_color_data_[0];
        }
    private:
        std::array<uint8_t, 16> regs_{};
        std::array<uint64_t, 32> screen_{};
        std::array<bool, 16> key_pressed_{};
        std::array<uint8_t, 4 * 64 * 32> screen_color_data_{};
        uint16_t i_ = 0;
        uint8_t dt_ = 0;
        uint8_t st_ = 0;
        std::array<uint16_t, 16> stack_;
        std::array<uint8_t, 0x1000> mem_;
        uint16_t pc_ = 0x200;
        uint8_t sp_ = 0;
        bool wait_keypress_ = false;
        size_t wait_reg_ = 0;
        void clear_screen();
        void redraw(size_t line_start, size_t lines, size_t x_start);
        void reset();
        inline void run_8inst(const Opcode& opcode);
        inline void run_Finst(const Opcode& opcode);
        bool load_file(const std::string& path);
        uint8_t read(uint16_t addr);
        void write(uint16_t addr, uint8_t data);
        Opcode get_next_opcode() {
            Opcode ret;
            ret.full = (mem_[pc_ & 0xFFF] << 8) | (mem_[(pc_ + 1) & 0xFFF]);
            pc_ += 2;
            return ret;
        }
        void run(Opcode opcode);
        void check_timers();
        std::chrono::system_clock::time_point timer = std::chrono::system_clock::now();
        friend class hydra::c8::Chip8_TKPWrapper;
    };
}
#endif