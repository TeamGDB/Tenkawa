# Tenkawa

A native port of a PlayStation Portable fighting game, built on [PortableKit](https://github.com/TeamGDB/PortableKit): the game's own code is translated to C++ ahead of time and compiled for your machine, and the console it ran on is reimplemented around it. It is not an emulator, and it is not a decompilation.

## Status

Nothing works yet. This repository is the starting point: the plan is a profile on top of PortableKit, the same way [Yakumo](https://github.com/TeamGDB/Yakumo) is.

## What a port needs

- The game's executable and its runtime code overlays, recompiled ahead of time.
- The system calls this game makes, which are not the same set another game makes.
- Whatever it asks of the graphics hardware that no earlier port needed.
- Its save format, its ad hoc multiplayer, and its own quirks.

## Game data

This project contains no game data and never will. A port reads a player's own legally obtained copy at first run.

## Legal

Not affiliated with or endorsed by any console maker or game publisher. All trademarks belong to their owners.

MIT licensed; see [LICENSE](LICENSE).
