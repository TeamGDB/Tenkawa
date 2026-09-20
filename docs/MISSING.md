# What is missing

What this port does not have yet, most blocking first. Everything here was measured, not guessed: the import lists come from the game's own import table, the recompiler's gaps from the code it generated, and the boot failures from running the game.

Platform for everything below: macOS on Apple Silicon, Vulkan through MoltenVK.

## Where the port is

The game's executable is prepared from the disc image, loaded and started. `module_start` runs, the game creates its own threads, and it reaches its frame loop and advances through it: 191,823,988 vblanks in a 150-second run.

It then stops making progress in a very specific way. Traced with `TENKAWA_TRACE_IO=1`:

```
[io] getstat disc0:/PSP_GAME/USRDIR/PACKFILE.BIN -> 0x00000000
[io] open umd1: flags=0x00000001 -> 0x00000003
[io] lseek fd=3 umd1: -> 56048
[io] open umd1: flags=0x00000001 -> 0x00000004
...
```

Three and a half million times in two minutes, never closing one. The game reads its data through the raw UMD device, and the framework's I/O layer has no answer for a disc device opened with no path — the first port's game used `sce_lbn` pseudo-paths instead. **This is the single thing now between this port and doing something visible**, and it is [TeamGDB/PortableKit#15](https://github.com/TeamGDB/PortableKit/issues/15).

Five calls were implemented in the framework to get this far, each one found by running the game and reading the last unimplemented call before it stopped: `sceKernelExtendThreadStack`, the lightweight mutexes, `sceKernelCheckThreadStack`, `sceKernelMemcpy`, and the three `sceDisplayWaitVblankStart` forms with `sceDisplayGetVcount`. None of them mentions this game; they are all things the first port's game happened not to need.

## 1. System calls the game makes that are not implemented

The game imports **228 functions from 25 libraries. 60 of them still have no implementation** and are bound to a logging stub that prints the call once and returns 0. A stub that returns 0 is a lie, and the game acts on it, so these are the first thing to work through.

The rest, by area. Everything implemented so far is struck from these tables.

### Blocking, or likely to be

| Library | Missing | Why it matters |
| --- | --- | --- |
| `IoFileMgrForUser` | Opening a disc device with no path, and `sceIoIoctl` | **The current blocker.** See above |
| `ThreadManForUser` (8 of 31) | `sceKernelCreateMbx`, `DeleteMbx`, `SendMbx`, `PollMbx`, `ReceiveMbxCB`; `sceKernelWaitThreadEnd`, `…EndCB`; `sceKernelWaitSemaCB` | Mailboxes are a whole IPC primitive the framework does not have. The `…CB` variants are the callback-polling forms of waits that do exist |
| `UtilsForUser` (3 of 7) | `sceKernelDcacheWritebackAll`, `…InvalidateAll`, `…Range` | No-ops on this host, but the game calls them before handing buffers to the GE, so they must at least return |

### Audio

| Library | Missing | Notes |
| --- | --- | --- |
| `sceSasCore` (13 of 27) | `__sceSasSetADSR`, `SetADSRmode`, `SetSL`, `GetEnvelopeHeight`, `GetAllEnvelopeHeights`, `SetGrain`, `GetGrain`, `SetNoise`, `SetOutputmode`, `SetVoicePCM`, `SetPause`, `GetPauseFlag`, `CoreWithMix`, and the four `__sceSasRev*` reverb calls | The mixer's envelope, grain, noise and reverb control. The first port never used them; this game drives all of it |
| `sceAudio` (4 of 12) | `sceAudioOutput2Reserve`, `…OutputBlocking`, `…Release`, `sceAudioOutputBlocking` | The Output2 single-channel streaming path, which the framework does not implement at all |
| `sceAtrac3plus` (4 of 5) | `sceAtracLowLevelInitDecoder`, `sceAtracLowLevelDecode`, `sceAtracGetAtracID`, `sceAtracReinit` | This game feeds the decoder raw frames itself instead of handing it a file, which is a different path through ATRAC3 than the one that exists |

### Video

| Library | Missing | Notes |
| --- | --- | --- |
| `sceMpeg` (4 of 23) | `sceMpegInit`, `sceMpegFinish`, `sceMpegAvcDecode`, `sceMpegAvcDecodeStop` | The framework demuxes and decodes audio but leaves AVC video to a different call than this game uses |

### Multiplayer

| Library | Missing | Notes |
| --- | --- | --- |
| `sceNetAdhocMatching` (11 of 11) | The entire library: `Init`, `Term`, `Create`, `Delete`, `Start`, `Stop`, `SelectTarget`, `CancelTargetWithOpt`, `SetHelloOpt`, `SendData`, `AbortSendData` | Peer matching. The first port's game did its own matchmaking over `sceNetAdhocctl` and never touched this |
| `sceNetAdhocctl` (4 of 12) | `sceNetAdhocctlConnect`, `Join`, `GetState`, `GetPeerInfo` | Joining and creating a group, and reading its state |

### Save data and shell

| Library | Missing | Notes |
| --- | --- | --- |
| `sceUtility` (6 of 21) | The five `sceUtilityGamedataInstall*` calls, and `sceUtilityGetSystemParamString` | This game installs data to the memory stick through the shell's own dialog. The executable names `ms0:/PSP/SAVEDATA/ULES01456INST/DATAINST.BIN`, and the disc holds a 600 MB `INSDIR/DATAINST.BIN`, so this is probably on the path to a first launch |
| `sceUmdUser` (3 of 5) | `sceUmdRegisterUMDCallBack`, `UnRegister…`, `sceUmdWaitDriveStatCB` | Disc-change callbacks |
| `sceImpose` (1 of 2) | `sceImposeSetUMDPopup` | Almost certainly safe to accept and ignore |
| `scePower` (1 of 4) | `scePowerUnregisterCallback` | Same |
| `ModuleMgrForUser` (2 of 5) | `sceKernelStopModule`, `sceKernelUnloadModule` | Only matters if the game loads a module at run time |

## 2. The recompiler

The whole executable recompiles: 8579 function seeds, 488049 code addresses, 153 C++ units, in about two and a half minutes. **435 of those 488049 addresses could not be lowered**, which is 0.09%. They are reached, if at all, through the interpreter, so they are not fatal — but each one is a place where a recompiled run silently becomes an interpreted one.

| Category | Sites | What they are |
| --- | --- | --- |
| `vfpu4 not lowered yet` | 390 | 24 distinct instruction words, of which one, `0xD0210000`, accounts for 359 sites spread through 0x08882C00–0x088FB344. The others are two small families, `0xD03Fxxxx` (22 sites) and `0xD05Bxxxx` (8) |
| `guest break trap` | 41 | `break` instructions: the game's own assertion traps. Correct as they are |
| `unknown not lowered yet` | 4 | All four are `addi` (opcode 0x08), the trapping add-immediate. The recompiler lowers `addiu` but not this |

The VFPU words need decoding against the hardware reference before anything is implemented; they are recorded here as words, not as guesses at what they do.

## 3. Graphics

**Still not known.** The game reaches its frame loop but has not submitted a display list, because it never gets its data off the disc. Nothing can be said about what it asks of the GE until it does, and this section stays empty rather than being filled with guesses.

## 4. Save data

The game writes `ms0:/PSP/SAVEDATA/ULES01456/…` and `ULES01456INST/DATAINST.BIN`. The framework implements the PSP save-data format — the `PARAM.SFO` layout, the encryption of the data file and the hashes that protect it — so saves should be exchangeable with a PSP. **Unverified:** no save has been made or read, the data file's name is not known, and whether this game encrypts its save at all has not been checked.

## 5. Not verified at all

- Anything on Linux or Windows. Everything here is macOS on Apple Silicon.
- Speed. The game's frame loop advances, but it draws nothing, so the numbers mean nothing yet. Guest time ran about 20,000x faster than real time in a headless run, which is what a loop with no work in it and no renderer to throttle it looks like.
- The ad hoc product code in the profile is a guess; the game's own is not known.
- That the game has no code overlays. The profile declares none, on the grounds that nothing has suggested otherwise, which is not the same as having looked.
