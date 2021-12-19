# TKPEmu
A multi-purpose emulator    
Using [Dear ImGui](https://github.com/ocornut/imgui) ([License](https://raw.githubusercontent.com/ocornut/imgui/master/LICENSE.txt))

**Warning:** If you somehow stumble upon this project this early in development, tread carefully.

## Current emulators
- [Gameboy](https://github.com/OFFTKP/TKPEmu/tree/master/TKPEmu/gb_tkp)

## Installation
### Archlinux, [AUR](https://aur.archlinux.org/packages/tkpemu/):
Installation for Archlinux is very easy, just run the following command:
`yay -S tkpemu`
Make sure you have [yay](https://github.com/Jguer/yay) installed.
Otherwise run the following:
```
pacman -S --needed git base-devel
git clone https://aur.archlinux.org/tkpemu.git
cd tkpemu
makepkg
```

### Windows:
Has not been tested.    
 - Download CMake for Windows and try to build.    
 - Create an issue if there's an error.

## License
TKPEmu is licensed under the MIT license. Copyright (C) 2021-2021 Paris Oplopoios
