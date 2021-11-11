# TKPEmu
A multi-purpose emulator    
Using [Dear ImGui](https://github.com/ocornut/imgui) ([License](https://raw.githubusercontent.com/ocornut/imgui/master/LICENSE.txt))

**Warning:** If you somehow stumble upon this project this early in development, tread carefully.

## Current emulators
- [Gameboy](https://github.com/OFFTKP/TKPEmu/tree/master/TKPEmu/Gameboy)

## Getting started
### Archlinux, using gcc, temporary solution before adding to AUR
- Clone the project
- Download [GLAD](https://glad.dav1d.de/) (select latest opengl version), add /include to your include path and add /src/glad.c to the project
- `pacman -S boost sdl2`
- Make sure `linux` preprocessor definition is defined
- Necessary arguments: `-std=c++20 -pthread -ldl -lSDL2 -ltbb`
- Compile main.cpp and all .cpp files in every subdirectory

## License
TKPEmu is licensed under the MIT license. Copyright (C) 2021-2021 Paris Oplopoios
