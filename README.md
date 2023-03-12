# Broken The Crew 2

<!-- [![](https://tokei.rs/b1/github/BrokenGameNoob/BrokenTC2?category=code)](https://github.com/XAMPPRocky/tokei).[![](https://tokei.rs/b1/github/BrokenGameNoob/BrokenTC2?category=lines)](https://github.com/XAMPPRocky/tokei).[![](https://tokei.rs/b1/github/BrokenGameNoob/BrokenTC2?category=files)](https://github.com/XAMPPRocky/tokei) -->

<!-- Badge help: -->
<!-- https://shields.io/category/size -->

![Repo size](https://img.shields.io/github/repo-size/BrokenGameNoob/BrokenTC2?style=for-the-badge) &nbsp;&nbsp; 

![Latest release](https://img.shields.io/github/v/release/BrokenGameNoob/BrokenTC2?label=Latest%20version&style=for-the-badge) 
<!-- ![GitHub release downloads (latest by date and asset)](https://img.shields.io/github/downloads/BrokenGameNoob/BrokenTC2/latest/BrokenTC2_setup_x64.exe?label=Was%20downloaded&style=for-the-badge) -->



## Features
- Sequential clutch (only 2 buttons required to go from Reverse to 7th gear as fast as possible)
- Current gear engaged by the software displayed on screen (allows to spot for differences between IG gear and BTC2 gear in case of shifting failure)
- Usable with wheels, controllers and pretty much anything recognized
- Profiles linked to controller in use
- Easy configuration and detailed instructions just below
- Automatic upgrades

## Download

[Click here to access release page](https://github.com/BrokenGameNoob/BrokenTC2/releases)

## Select language
- [Français](https://github.com/BrokenGameNoob/BrokenTC2/blob/master/Docs/README_fr.md)
- [English - auto translated via google translate](https://github-com.translate.goog/BrokenGameNoob/BrokenTC2/blob/master/Docs/README_fr.md?_x_tr_sl=fr&_x_tr_tl=en&_x_tr_hl=fr&_x_tr_pto=wapp)

<br/><br/><br/><br/>

## ---- Build ----

<br/><br/>

### Setup

<br/>

#### SDL

Setup SDL by downloading the `SDL2-devel-{version}-mingw` package from [github/SDL](https://github.com/libsdl-org/SDL/releases)
Then extract the folder `x86_64-w64-mingw32` into `C:/Libs/` so that the final include directory for the SDL is `C:/Libs/x86_64-w64-mingw32/include`

<br/>

#### Protobuf

1. Clone the Protobuf repo in the dir you would like to install it. DOT NOT FORGET SUBMODULES
```bash
git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf
git checkout v22.2
git switch -c release22.2
git submodule init
git submodule update
mkdir build && cd build
```
2. Configure project
```bash
cmake -Dprotobuf_BUILD_TESTS=OFF -DCMAKE_C_COMPILER:FILEPATH=C:\\Dev\\mingw64\\bin\\gcc.exe -DCMAKE_CXX_COMPILER:FILEPATH=C:\\Dev\\mingw64\\bin\\g++.exe -G "MinGW Makefiles" ../cmake
```
> ***Note***: Replace the path to compilers with yours
3. Build & install
```bash
cmake --build .
cmake --build . -j 16
cmake --build . --target install
```
> ***Note***: The install command may need to be run using admin privileges
4. Copy the built folder (`C:/Program Files (x86)/protobuf` by default) to somewhere **not containing spaces**

<br/><br/>

### CMake configuration

Useful variables:
- `SDL2_BASE_INSTALLATION_PATH`: base SDL2 installation path. This folder shall contains the following subfolders: `include`, `lib`, `share`, `bin`
    > ***Note***: default is `C:\\Libs\\x86_64-w64-mingw32`
- `Protobuf_BASE_INSTALLATION_PATH`: base protobuf installation path. This folder shall contains the following subfolders: `include`, `lib`, `bin`
    > ***Note***: default is `C:\\Libs\\protobuf`