[![tkpemu](https://img.shields.io/aur/version/tkpemu?color=1793d1&label=yay&logo=arch-linux&style=for-the-badge)](https://aur.archlinux.org/packages/tkpemu)
[![tkpemu-votes](https://img.shields.io/aur/votes/tkpemu?color=333333&style=for-the-badge)](https://aur.archlinux.org/packages/tkpemu)
[![GitHub license](https://img.shields.io/github/license/offtkp/tkpemu?color=333333&style=for-the-badge)](https://github.com/offtkp/tkpemu/blob/master/LICENSE)
# TKPEmu
A multi-purpose, multi-platform emulator    
Using [Dear ImGui](https://github.com/ocornut/imgui) ([License](https://raw.githubusercontent.com/ocornut/imgui/master/LICENSE.txt))    
![Image](./TKPEmu/screen.png)

## Current emulators
- [Gameboy](https://github.com/OFFTKP/TKPEmu/tree/master/TKPEmu/gb_tkp)
## Dependencies 
Compiler: The `c++20` features used need at least `gcc-11` and `g++11` or latest `msvc`.   
Dependencies: `cmake git `    
Libraries: `sdl2 tbb boost`. See `Installation` for an easy installation guide

## Installation
CMake is going to download source files from [imgui](https://github.com/ocornut/imgui), [GameboyTKP](https://github.com/OFFTKP/GameboyTKP) and [glad-stable](https://github.com/OFFTKP/glad-stable)    
If you get an error while these files are being downloaded, check if the links above work, and open an issue
### Archlinux, [AUR](https://aur.archlinux.org/packages/tkpemu/):
Installation for Archlinux is very easy, just run the following command:    
```
yay -S tkpemu
```   
Make sure you have [yay](https://github.com/Jguer/yay) installed.    
Otherwise run the following:    
```
pacman -S --needed git base-devel
git clone --recurse-submodules -j8 https://aur.archlinux.org/tkpemu.git
cd tkpemu
makepkg -si
```
### Ubuntu:
```
sudo apt-get update
sudo apt-get install libsdl2-dev libtbb-dev libboost-all-dev build-essential gcc-11 g++-11
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 --slave /usr/bin/g++ g++ /usr/bin/g++-11
sudo update-alternatives --set gcc /usr/bin/gcc-11
git clone --recurse-submodules -j8 https://github.com/OFFTKP/TKPEmu.git
cd TKPEmu
cmake -S TKPEmu -B TKPEmu/build
cmake --build TKPEmu/build
sudo mv ./TKPEmu/build/TKPEmu /usr/bin/TKPEmu
```

### Windows:
Has not been tested.    
 - Download CMake for Windows and try to build.    
 - Create an issue if there's an error.

## License
TKPEmu is licensed under the MIT license    
Copyright (C) 2021-2021 Paris Oplopoios

## Contributors
[![GitHub contributors](https://contrib.rocks/image?repo=OFFTKP/TKPEmu)](https://github.com/OFFTKP/TKPEmu/graphs/contributors)
