<!-- [![hydra](https://img.shields.io/aur/version/hydra?color=1793d1&label=yay&logo=arch-linux&style=for-the-badge)](https://aur.archlinux.org/packages/hydra) -->
<!-- [![GitHub license](https://img.shields.io/github/license/offtkp/hydra?color=333333&style=for-the-badge)](https://github.com/offtkp/hydra/blob/master/LICENSE)
![Commit activity](https://img.shields.io/github/commit-activity/m/OFFTKP/hydra?style=for-the-badge)
![Stars](https://img.shields.io/github/stars/OFFTKP/hydra?style=for-the-badge)
![Size](https://img.shields.io/github/repo-size/OFFTKP/hydra?style=for-the-badge) -->
<p align="center"> <img src="./data/images/hydra.png"><img src="./data/images/logo.png"></p>

----

<p align="center">A multi-system emulator</p>
<p align="center"><img src="./data/images/screen.png"></p>

## Current systems
- Gameboy / Gameboy Color
- Chip 8
- NES (WIP)
- Nintendo 64 (WIP)

## Dependencies 
Compiler: The `c++20` features used need at least `gcc-11` and `g++11` or latest `msvc`.   
Dependencies: `cmake git `    
Libraries: `sdl2 tbb qt`. See `Installation` for an easy installation guide

## Installation

<details>
 <summary>Archlinux</summary>
<pre><code>pacman -S --needed git cmake sdl2 glfw-x11 ninja qt
git clone https://github.com/OFFTKP/hydra.git
cd hydra
cmake -S hydra -B hydra/build -G Ninja
cmake --build hydra/build
</code></pre>
</details>
<details>
<summary>Ubuntu</summary><br>
TODO: wrong and old, fix
These commands are used to install on a fresh ubuntu environment and some can be omitted.
<pre><code>sudo apt-get update
sudo apt-get install libsdl2-dev libtbb-dev libboost-all-dev build-essential gcc-11 g++-11 ninja-build
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 --slave /usr/bin/g++ g++ /usr/bin/g++-11
sudo update-alternatives --set gcc /usr/bin/gcc-11
git clone https://github.com/OFFTKP/hydra.git
cd hydra
cmake -S hydra -B hydra/build
cmake --build hydra/build
</code></pre>
</details>
<details>
<summary>Windows</summary><br>
Has not been tested. Follow similar procedure, clone and build with cmake.
</details>

## Contributing
Any contribution is welcome. Fork, open an issue or work on existing ones, or work on improving the documentation

## License
hydra is licensed under the MIT license    
Copyright (C) 2021-2023 Paris Oplopoios

## Contributors
[![GitHub contributors](https://contrib.rocks/image?repo=OFFTKP/hydra)](https://github.com/OFFTKP/hydra/graphs/contributors)
