# GameboyTKP
Gameboy emulator written in C++ for [TKPEmu](https://github.com/OFFTKP/TKPEmu).
## Features

 - Disassembler/Debugger with step/reset/pause functionality
 - Advanced breakpoints. You can create a breakpoint for a specific instruction with specific register values
 - Real time register values shown on disassembler
 - Trace logger (WIP)
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
| bits/mem_oam | ✅ |
| bits/reg_f | ✅ |
| bits/unused_hwio_GS | ✅ |
| instr/daa | ✅ |
| interrupts/ie_push | ❌ |
| oam_dma/basic | ❌ |
| oam_dma/reg_read | ✅ |
| oam_dma/sources-GS | ❌ (needs MBC5)|
| oam_dma/sources-GS | ❌ (needs MBC5)|
| ppu/... | ❌ (untested)|
| serial/... | ❌ (untested)|
| timer/div_write | ✅ |
| timer/rapid_toggle | ❌ |
| timer/tim00 | ✅ |
| timer/tim00_div_trigger | ✅ |
| timer/tim01 | ❌ (Issue [#4](https://github.com/OFFTKP/TKPEmu/issues/4))|
| timer/tim01_div_trigger | ✅ |
| timer/tim10 | ✅ |
| timer/tim10_div_trigger | ✅ |
| timer/tim11 | ✅ |
| timer/tim11_div_trigger | ✅ |
| timer/tima_reload | ❌ |
| timer/tima_write_reloading | ❌ |
| timer/tma_write_reloading | ❌ |
Note: any test that isn't listed here is untested.

**[mattcurie](https://github.com/mattcurrie)'s tests:**
|Test|GameboyTKP  |
|--|--|
| dmg-acid2 | ❌ (Milestone [#1](https://github.com/OFFTKP/TKPEmu/milestone/1)) |

## License
See [TKPEmu](https://github.com/OFFTKP/TKPEmu) license
