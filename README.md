# Tenkawa

A native port of **Dragon Ball Z: Tenkaichi Tag Team** (PlayStation Portable) made by static recompilation: the game's PSP code is translated ahead of time into C++ and compiled for your machine, then run on a reimplementation of the PSP system software. It is not an emulator — there is no interpreter or JIT at the heart of it — and it is not a decompilation.

It is built on [PortableKit](https://github.com/TeamGDB/PortableKit), the shared recompiler and runtime, the same way [Yakumo](https://github.com/TeamGDB/Yakumo) is.

> **This project does not include any game assets.** You must provide the files from your own legally obtained copy of Dragon Ball Z: Tenkaichi Tag Team to install or build Tenkawa.

> It is not an emulator in the sense of a JIT or an interpreter at the heart of it. There *is* an interpreter, and early in a port's life it does the work: it runs the game before any of its code has been recompiled, which is how you find out what the game needs without waiting hours for a recompile first.

## Status

The game does not run yet.

What works: the installer accepts the disc image, checks it and decrypts the game's executable; the executable loads; `module_start` runs. What does not: everything after that. The game has not created its own main thread, drawn a frame or made a sound.

What is measured so far:

| | |
| --- | --- |
| Imports the game makes | 228, from 25 libraries |
| Of those, with no implementation | 71 |
| Recompiled | the whole executable: 8579 functions, 488049 addresses, 153 C++ units |
| Addresses the recompiler could not lower | 435, or 0.09% — mostly one VFPU instruction |

[`docs/MISSING.md`](docs/MISSING.md) is the list, most blocking first, and the [issues](https://github.com/TeamGDB/Tenkawa/issues) are the work.

## How the port is built

This repository is a **profile**: one file of constants describing the game, and a four-line `CMakeLists.txt`. Everything else — the recompiler, the kernel, the system modules, the Vulkan renderer, audio, save data, ad hoc networking, the interface and the installer — is [PortableKit](https://github.com/TeamGDB/PortableKit), included here as a submodule.

```bash
git clone --recursive https://github.com/TeamGDB/Tenkawa.git
cd Tenkawa
cmake -S . -B out -G Ninja
cmake --build out -j2
out/bin/TenkawaNative --install /path/to/your/image.iso
out/bin/TenkawaNative
```

Keep `-j` low: each recompiled unit needs more than a gigabyte to compile. PortableKit's [`docs/BUILDING.md`](https://github.com/TeamGDB/PortableKit/blob/main/docs/BUILDING.md) explains where the time goes, and [`docs/BRINGING_UP_A_GAME.md`](https://github.com/TeamGDB/PortableKit/blob/main/docs/BRINGING_UP_A_GAME.md) explains how to work on a port that does not run yet.

## Legal disclaimer

**Tenkawa** does not include any game assets or original game files: no disc image, no copy of the game's executable or data, and no textures, models, audio or video from the game. You must provide the files from your own legally obtained copy of Dragon Ball Z: Tenkaichi Tag Team to install or build **Tenkawa**; the installer checks that copy and accepts only the original release.

Users are solely responsible for obtaining, dumping, extracting, and using their game copy in accordance with the laws applicable in their jurisdiction.

**Tenkawa** does not support, provide, link to, or encourage the use of unauthorized or pirated copies of the game.

Any references to the original game or its trademarks are made solely for identification, compatibility, and interoperability purposes.

Screenshots and other depictions of the original game may be used solely to document or demonstrate **Tenkawa's** functionality. All depicted third-party game content remains the property of its respective rights holders.

The license covering **Tenkawa** applies only to the project's own original code and materials and does not grant any rights to third-party intellectual property.

**Tenkawa** provides the software, not the game. You must provide your own legally obtained copy.

MIT licensed; see [LICENSE](LICENSE).
