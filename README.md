<!-- [![hydra](https://img.shields.io/aur/version/hydra?color=1793d1&label=yay&logo=arch-linux&style=for-the-badge)](https://aur.archlinux.org/packages/hydra) -->
<!-- [![GitHub license](https://img.shields.io/github/license/offtkp/hydra?color=333333&style=for-the-badge)](https://github.com/offtkp/hydra/blob/master/LICENSE)
![Commit activity](https://img.shields.io/github/commit-activity/m/OFFTKP/hydra?style=for-the-badge)
![Stars](https://img.shields.io/github/stars/OFFTKP/hydra?style=for-the-badge)
![Size](https://img.shields.io/github/repo-size/OFFTKP/hydra?style=for-the-badge) -->
<p align="center"> <img src="./data/images/hydra.png"><img src="./data/images/logo.png"></p>

----

<p align="center">A multi-system emulator</p>
<p align="center"><img src="./data/images/screen.png"></p>

## What is this?
This is the Hydra, a multi-platform frontend for emulator cores.
A Hydra core is a shared library that defines the functions in the [core header](https://github.com/hydra-emu/core).
You can load cores by placing them in the core directory specified in Hydras settings.

## Current cores
- Nintendo 64
  - [n64hydra](https://github.com/hydra-emu/n64)
- Gameboy Color
  - [SkyEmu](https://github.com/hydra-emu/SkyEmu)
- Gameboy Advance
  - [SkyEmu](https://github.com/hydra-emu/SkyEmu)
- Nintendo DS
  - [SkyEmu](https://github.com/hydra-emu/SkyEmu)
  - [vargds](https://github.com/hydra-emu/vargds) (WIP)
- Nintendo 3DS
  - [Panda3DS](https://github.com/hydra-emu/Panda3DS)

## Building

You will need a C++20 compliant compiler like `gcc-12`

<details>
 <summary>Archlinux</summary>
<pre><code>pacman -S --needed qt6
git clone https://github.com/OFFTKP/hydra.git
cd hydra
cmake -B build
cmake --build build --target hydra -j $(nproc)
</code></pre>
</details>
<details>
<summary>Ubuntu</summary><br>
<pre><code>sudo apt-get update
sudo apt-get install libgl-dev qt6-base-dev libqt6openglwidgets6 libqt6widgets6 libqt6opengl6 libqt6gui6
git clone https://github.com/OFFTKP/hydra.git
cd hydra
cmake -B build
cmake --build build --target hydra -j $(nproc)
</code></pre>
</details>
<details>
<summary>MacOS</summary><br>
You can replace <code>-j 4</code> with your actual number of cores
<pre><code>brew install qt6
cmake -B build
cmake --build build --target hydra -j 4
</code></pre>
</details>
<details>
<summary>Windows</summary><br>
Currently does not pass CI, so compilation might fail
Make sure to install Qt6 first
<pre><code>cmake.exe -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake.exe --build build --target hydra -j %NUMBER_OF_PROCESSORS%
</code></pre>
</details>

## Sister projects
[shadPS4](https://github.com/georgemoralis/shadPS4): Work-in-progress PS4 emulator by the founder of PCSX, PCSX2 and more    
[Panda3DS](https://github.com/wheremyfoodat/Panda3DS): A new, panda-themed, HLE Nintendo 3DS emulator

## Contributing
Any contribution is welcome. Fork, open an issue or work on existing ones, or work on improving the documentation

## Contributors
[![GitHub contributors](https://contrib.rocks/image?repo=OFFTKP/hydra)](https://github.com/OFFTKP/hydra/graphs/contributors)
