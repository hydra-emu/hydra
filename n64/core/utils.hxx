#ifndef UTILS_HXX
#define UTILS_HXX
#include <string>
#include <concepts>

template<class T>
requires std::integral<T>
static void SetBit(T& num, int bit, bool value) {
    num ^= (-value ^ num) & (1UL << static_cast<int>(bit));
}

static std::string CP0String(int reg) {
    switch (reg) {
        #define X(name, reg) case reg: return "CP0_"#name;
        #include "cp0_regs.def"
        #undef X
    }
    return "Unknown CP0 register";
}
#endif