# TKPEmu
A multi-purpose emulator    
Using [Dear ImGui](https://github.com/ocornut/imgui) ([License](https://raw.githubusercontent.com/ocornut/imgui/master/LICENSE.txt))

**Warning:** If you somehow stumble upon this project this early in development, tread carefully.

## Current emulators
- [Gameboy](https://github.com/OFFTKP/TKPEmu/tree/master/TKPEmu/Gameboy)

## Getting started
### Archlinux, using gcc, temporary solution before adding to AUR
- Clone the project
- `pacman -S boost sdl2`
- Run `cmake -S . -D /bin`
- `cd bin`
- `make`
- `./TKPEmu -h` to list commands

## License
TKPEmu is licensed under the MIT license. Copyright (C) 2021-2021 Paris Oplopoios
