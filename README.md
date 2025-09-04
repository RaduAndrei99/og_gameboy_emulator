# OG Gameboy emulator.

## Documentation

The following resources helped developing this project:

General doc: [gbdev.io](https://gbdev.io/pandocs/About.html)

SM83 instruction set table: [SM83](https://gbdev.io/gb-opcodes/optables/)

SM83 instruction set in detail: [SM83](https://rgbds.gbdev.io/docs/v0.9.4/gbz80.7)

## Prerequisites

- **C++14** compatible compiler (GCC, Clang, or MSVC)  
- **CMake** 3.15 or newer  
- **Git** (optional, for cloning) 

## Building

```sh
mkdir build
```

```sh
cd build
```

```sh
cmake ..
```

```sh
cmake --build .
```
## Running

```sh
./gbemu
```