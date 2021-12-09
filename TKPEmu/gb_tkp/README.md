# GameboyTKP
Gameboy emulator written in C++ for [TKPEmu](https://github.com/OFFTKP/TKPEmu).
## Features

 - Disassembler/Debugger with step/reset/pause functionality
 - Advanced breakpoints. You can create a breakpoint for a specific instruction with specific register values
 - Real time register values shown on disassembler
 - Trace logger
 - Save states (WIP)
 - Rewind functionality (WIP)
## Tests

**[Blargg](https://github.com/gblargg)'s tests:**
| Test | GameboyTKP  |
|--|--|
| cpu_instrs | ✅ |
| instr_timing | ❌ (Issue [#3](https://github.com/OFFTKP/TKPEmu/issues/3))|
| mem_timing | ❌ |
| dmg_sound | ❌ |
| oam_bug | ❌ |

**[Gekkio](https://github.com/Gekkio)'s acceptance tests:**

|Test| GameboyTKP |
|--|--|
| acceptance/div_timing | ✅ |
| acceptance/if_ie_registers | ✅ |
| acceptance/boot_regs-dmgABC | ✅ |
| acceptance/bits/mem_oam | ✅ |
| acceptance/bits/reg_f | ✅ |
| acceptance/bits/unused_hwio_GS | ✅ |
| acceptance/instr/daa | ✅ |
| acceptance/interrupts/ie_push | ❌ |
| acceptance/oam_dma/basic | ❌ |
| acceptance/oam_dma/reg_read | ✅ |
| acceptance/ppu/... | ❌ (untested)|
| acceptance/serial/... | ❌ (untested)|
| acceptance/timer/div_write | ✅ |
| acceptance/timer/rapid_toggle | ❌ |
| acceptance/timer/tim00 | ✅ |
| acceptance/timer/tim00_div_trigger | ❌ |
| acceptance/timer/tim01 | ✅ |
| acceptance/timer/tim01_div_trigger | ❌ |
| acceptance/timer/tim10 | ✅ |
| acceptance/timer/tim10_div_trigger | ❌ |
| acceptance/timer/tim11 | ✅ |
| acceptance/timer/tim11_div_trigger | ❌ |
| acceptance/timer/tima_reload | ✅ |
| acceptance/timer/tima_write_reloading | ❌ |
| acceptance/timer/tma_write_reloading | ❌ |
| emulator-only/mbc1/bits_bank1 | ✅ | 
| emulator-only/mbc1/bits_bank2 | ✅ | 
| emulator-only/mbc1/bits_mode | ✅ | 
| emulator-only/mbc1/bits_ramg | ❌ | 
| emulator-only/mbc1/multicart_rom_8Mb | ❌ | 
| emulator-only/mbc1/ram_256kb | ❌ | 
| emulator-only/mbc1/ram_64kb | ❌ | 
| emulator-only/mbc1/rom_16Mb | ✅ | 
| emulator-only/mbc1/rom_1Mb | ❌ (multicart) | 
| emulator-only/mbc1/rom_2Mb | ❌ (multicart) | 
| emulator-only/mbc1/rom_4Mb | ✅ | 
| emulator-only/mbc1/rom_512kb | ❌ |
| emulator-only/mbc1/rom_8Mb | ✅ | 

**[mattcurie](https://github.com/mattcurrie)'s tests:**
|Test|GameboyTKP  |
|--|--|
| dmg-acid2 | ❌ (Milestone [#1](https://github.com/OFFTKP/TKPEmu/milestone/1)) |

## License
See [TKPEmu](https://github.com/OFFTKP/TKPEmu) license
