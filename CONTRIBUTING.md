# Contributing to Tenkawa

Thank you for wanting to help. Tenkawa is a work in progress; [`docs/MISSING.md`](docs/MISSING.md) says what is missing, most blocking first, and the [issues](https://github.com/TeamGDB/Tenkawa/issues) are the work. These rules are the short form of [AGENTS.md](AGENTS.md) and of [PortableKit's contributing guide](https://github.com/TeamGDB/PortableKit/blob/main/CONTRIBUTING.md), which apply here too.

## Where a change belongs

Almost nothing lives in this repository: it is the game's profile, `host/tenkawa_profile.cpp`. **A change that is true of the PSP, of a file format, or of any game that makes the same call belongs in [PortableKit](https://github.com/TeamGDB/PortableKit)**, as a pull request there. Only what names this game — its disc id, addresses, save folders, quirks — belongs here. An unimplemented system call is almost always the framework's.

## The rules

- **English only** in everything committed, and in issues and pull requests.
- **No game data, ever.** No disc images, executables (`EBOOT.BIN`, `BOOT.BIN`, decrypted ELFs, PRX modules), code generated from the game (`generated/`), saves, audio or video taken from the game, or screenshots and captures of its art. Not in commits, not in issues, not in pull requests, not as attachments. `.gitignore` covers `game/`, `generated/`, `overlays/`, `analysis/` and `out/`; do not work around it.
- **Nothing personal.** No home paths, user names, machine names, private e-mail addresses or IP addresses in anything committed or pasted. Check logs before you paste them.
- **Write it yourself.** Public documentation and observed behaviour are fine to learn from; code from a project whose licence is incompatible with MIT — PPSSPP and other GPL emulators included — must never be copied, adapted or translated line by line. Constants, offsets and format facts are fine.
- **Trace the game; don't recall the hardware.** GE register numbers, PSP struct layouts and system call semantics from memory have been wrong before. Turn on the relevant `TENKAWA_TRACE_*` switch, read what the game does, then write the code.
- **Bound every run.** `timeout 90 out/bin/TenkawaNative`; never leave the game running or drive it with an open-ended input loop.
- **Say what you did not verify.**

## How to send a change

1. Fork the repository and create a branch. Nobody pushes to `main`.
2. Build and run the port (see the [README](README.md)); a framework change also needs PortableKit's own tests to pass.
3. Open a pull request. Say what changed and why, how you verified it (platform, GPU, where in the game, what you looked at), what you did not verify, and which issues it closes. If it depends on a PortableKit pull request, link it.

## Licence

By contributing you agree that your contribution is licensed under the [MIT License](LICENSE), like the rest of the project.
