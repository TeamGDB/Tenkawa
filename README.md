# Tenkawa

A native port of **Dragon Ball Z: Tenkaichi Tag Team** (PlayStation Portable) made by static recompilation: the game's PSP code is translated ahead of time into C++ and compiled for your machine, then run on a reimplementation of the PSP system software. It is not an emulator — there is no interpreter or JIT at the heart of it — and it is not a decompilation.

It is built on [PortableKit](https://github.com/TeamGDB/PortableKit), the shared recompiler and runtime, the same way [Yakumo](https://github.com/TeamGDB/Yakumo) is.

> **This project does not include any game assets.** You must provide the files from your own legally obtained copy of Dragon Ball Z: Tenkaichi Tag Team to install or build Tenkawa.

## Status

Nothing works yet. This repository is the starting point.

## What this port needs

- The game's executable and any runtime code overlays, recompiled ahead of time.
- The system calls this game makes, which are not the same set another game makes.
- Whatever it asks of the graphics hardware that no earlier port needed.
- Its save format, its ad hoc multiplayer, and its own quirks.

## Legal disclaimer

**Tenkawa** does not include any game assets or original game files: no disc image, no copy of the game's executable or data, and no textures, models, audio or video from the game. You must provide the files from your own legally obtained copy of Dragon Ball Z: Tenkaichi Tag Team to install or build **Tenkawa**; the installer checks that copy and accepts only the original release.

Users are solely responsible for obtaining, dumping, extracting, and using their game copy in accordance with the laws applicable in their jurisdiction.

**Tenkawa** does not support, provide, link to, or encourage the use of unauthorized or pirated copies of the game.

Any references to the original game or its trademarks are made solely for identification, compatibility, and interoperability purposes.

Screenshots and other depictions of the original game may be used solely to document or demonstrate **Tenkawa's** functionality. All depicted third-party game content remains the property of its respective rights holders.

The license covering **Tenkawa** applies only to the project's own original code and materials and does not grant any rights to third-party intellectual property.

**Tenkawa** provides the software, not the game. You must provide your own legally obtained copy.

MIT licensed; see [LICENSE](LICENSE).
