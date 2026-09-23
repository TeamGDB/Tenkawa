# Tenkawa

A native port of **Dragon Ball Z: Tenkaichi Tag Team** (PlayStation Portable) made by static recompilation: the game's PSP code is translated ahead of time into C++ and compiled for your machine, then run on a reimplementation of the PSP system software. It is not an emulator — there is no JIT at the heart of it — and it is not a decompilation.

> **Work in progress — not playable yet.** Tenkawa is at the very start of its development. It boots and reaches the game's title screen, but it cannot be played, and you should expect crashes, missing features and breaking changes. It is published so that people can follow and help with the work, not to be played.

> **No game data is included.** This repository contains no disc image, no executable, no code generated from the game, and no textures, models, audio or video from it. To build or run Tenkawa you need your own legally obtained copy of Dragon Ball Z: Tenkaichi Tag Team — the European release, `ULES-01456` — as a disc image. Tenkawa is not affiliated with, endorsed by or sponsored by Sony Interactive Entertainment or the game's developers, publishers or licensors.

It is built on [PortableKit](https://github.com/TeamGDB/PortableKit), the shared recompiler and runtime, the same way [Yakumo](https://github.com/TeamGDB/Yakumo) and [Purun](https://github.com/TeamGDB/Purun) are.

There is an interpreter too, and early in a port's life it does the work: it runs the game before any of its code has been recompiled, which is how you find out what the game needs without waiting hours for a recompile first.

## Status

**It reaches the title screen, under the interpreter.** It installs from a disc image, boots, runs its threads, reads the disc, shows its clock-frequency notice and memory-stick check, and draws its title screen correctly. The **recompiled** build runs its frame loop at full speed but does not draw yet ([PortableKit#20](https://github.com/TeamGDB/PortableKit/issues/20)), and nothing past the title has been reached. [`docs/MISSING.md`](docs/MISSING.md) is the list, most blocking first, and the [issues](https://github.com/TeamGDB/Tenkawa/issues) are the work.

| | |
| --- | --- |
| Tested on | macOS on Apple Silicon, Vulkan through MoltenVK. **Linux, Windows and the Steam Deck have not been tried** |
| Imports the game makes | 228, from 25 libraries |
| Of those, with no implementation | run with `TENKAWA_LIST_STUBS=1` to see them |
| Recompiled | 8579 functions, 488049 addresses, 153 C++ units |
| Addresses the recompiler cannot lower | the game's own 41 `break` traps, and nothing else |

## Building

You need an ISO image of your own UMD disc of the European release (`ULES-01456`), made with your own PSP — for example with a homebrew UMD dumping tool, or a custom firmware's USB mode that exposes the disc. The installer checks the disc id and the SHA-256 of the executable and refuses anything else. Nobody here can give you a copy or tell you where to get one.

The tools are PortableKit's: a C++20 compiler, CMake 3.20+, Ninja, Python 3, `make`, SDL3, a Vulkan loader and headers, and `glslangValidator`; `ccache` is strongly recommended. On macOS: `brew install cmake ninja ccache python sdl3 molten-vk vulkan-loader vulkan-headers glslang`. [Purun's README](https://github.com/TeamGDB/Purun#what-you-need) lists the Linux and Windows equivalents, which are equally untested here.

```bash
git clone --recursive https://github.com/TeamGDB/Tenkawa.git
cd Tenkawa
cmake -S . -B out -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build out -j2                                     # the bootstrap: minutes
out/bin/TenkawaNative --install /path/to/your/image.iso   # check the disc and prepare the executable
TENKAWA_INPUT_SCRIPT="600:pad a;700:pad a" out/bin/TenkawaNative
```

That runs the game under the interpreter, which is how the port is worked on today; the two scripted presses get it past the first notice. `scripts/generate.sh out` followed by another `cmake --build out -j2` recompiles it, which takes hours and does not draw yet. Keep `-j` low: each recompiled unit needs more than a gigabyte to compile. PortableKit's [`docs/BUILDING.md`](https://github.com/TeamGDB/PortableKit/blob/main/docs/BUILDING.md) explains where the time goes, and [`docs/BRINGING_UP_A_GAME.md`](https://github.com/TeamGDB/PortableKit/blob/main/docs/BRINGING_UP_A_GAME.md) how to work on a port that does not run yet.

Settings, the prepared executable and saves live in the data directory: `~/Library/Application Support/Tenkawa/DBZTTT` on macOS, `~/.local/share/Tenkawa/DBZTTT` on Linux, `%APPDATA%\Tenkawa\DBZTTT` on Windows. Every environment variable takes the `TENKAWA_` prefix: `TENKAWA_DATA_DIR`, `TENKAWA_GAME_DIR`, `TENKAWA_NO_RENDER`, `TENKAWA_NO_AUDIO`, `TENKAWA_LIST_STUBS`, `TENKAWA_PERF=log`, `TENKAWA_TRACE_*`.

## Reporting bugs and helping

Open an [issue](https://github.com/TeamGDB/Tenkawa/issues/new/choose) with the bug report form: platform, the commit you built (`git describe --always --dirty`) and the log (`out/bin/TenkawaNative > run.log 2>&1`). **Never attach game files** — no disc images, executables, saves, generated code, or audio and video taken from the game. Contributions are welcome; read [CONTRIBUTING.md](CONTRIBUTING.md) first. Most of the work is in PortableKit.

## How the port is built

This repository is a **profile**: one file of constants describing the game (`host/tenkawa_profile.cpp`), and a four-line `CMakeLists.txt`. Everything else — the recompiler, the kernel, the system modules, the Vulkan renderer, audio, save data, ad hoc networking, the interface and the installer — is [PortableKit](https://github.com/TeamGDB/PortableKit), included here as the `portablekit` submodule.

## Legal disclaimer

**Tenkawa** does not include any game assets or original game files: no disc image, no copy of the game's executable or data, and no textures, models, audio or video from the game. You must provide the files from your own legally obtained copy of Dragon Ball Z: Tenkaichi Tag Team to install or build **Tenkawa**; the installer checks that copy and accepts only the original release.

Users are solely responsible for obtaining, dumping, extracting, and using their game copy in accordance with the laws applicable in their jurisdiction.

**Tenkawa** does not support, provide, link to, or encourage the use of unauthorized or pirated copies of the game.

**Tenkawa** is an independent project. It is not affiliated with, endorsed by or sponsored by Sony Interactive Entertainment, or by the developers, publishers or licensors of Dragon Ball Z: Tenkaichi Tag Team. PlayStation and PSP are trademarks of Sony Interactive Entertainment; Dragon Ball Z is a trademark of its owner. Any references to the original game or its trademarks are made solely for identification, compatibility, and interoperability purposes.

Screenshots and other depictions of the original game may be used solely to document or demonstrate **Tenkawa's** functionality. All depicted third-party game content remains the property of its respective rights holders.

The license covering **Tenkawa** applies only to the project's own original code and materials and does not grant any rights to third-party intellectual property.

**Tenkawa** provides the software, not the game. You must provide your own legally obtained copy.

Tenkawa's own code is MIT licensed; see [LICENSE](LICENSE). PortableKit and the third-party components it uses keep their own licences; see PortableKit's [`docs/SOURCE_PROVENANCE.md`](https://github.com/TeamGDB/PortableKit/blob/main/docs/SOURCE_PROVENANCE.md).
