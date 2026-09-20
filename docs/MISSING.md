# What is missing

What this port does not have yet, most blocking first. Everything here was measured, not guessed: the import lists come from the game's own import table, the recompiler's gaps from the code it generated, and the boot failures from running the game.

Platform for everything below: macOS on Apple Silicon, Vulkan through MoltenVK.

## Where the port is

The game starts, loads, and runs. `module_start` runs, the game creates its threads, reaches its frame loop and advances through it, reads its data off the disc, sets up audio, and keeps going with no deadlock and no starvation. A thread dump partway in looks like a working game:

```
threads (virtual time 133 ms, vblanks 8):
  uid=258  prio=0x0000003D running  pc=0x0881A250
  uid=269  prio=0x00000043 waiting  wait=mailbox object=267 deadline=168ms pc=0x08831C8C
  uid=280  prio=0x00000010 waiting  wait=delay   object=0   deadline=156ms pc=0x0883E554
```

It stops on **a VFPU instruction neither the recompiler nor the interpreter can execute**:

```
Runtime stopped: Unsupported Allegrex instruction 0xD03CA084 at 0x0882EAD4: vfpu4 not lowered yet
```

That is [TeamGDB/PortableKit#3](https://github.com/TeamGDB/PortableKit/issues/3), and it is now the blocker. Nothing has been drawn yet, so there is still nothing to screenshot.

### What has been implemented to get here

Nine framework calls and two framework fixes, each one found by running the game and reading what it stopped on. None of them mentions this game:

| What | Why it was in the way |
| --- | --- |
| `sceKernelExtendThreadStack` | Must call the function it is given; stubbed, `module_start` returned without creating the game's main thread |
| Lightweight mutexes (create/delete, lock/unlock) | Used throughout start-up |
| `sceKernelCheckThreadStack` | Returning 0 says the stack is exhausted, and the game believes it |
| `sceKernelMemcpy` | Returning without copying corrupts what was to be copied |
| `sceDisplayWaitVblankStart` ×3, `sceDisplayGetVcount` | Nothing paced the frame loop, so it spun |
| Raw disc device, in sectors | The game reads `PACKFILE.BIN` straight off the disc |
| Kernel mailboxes | Created during start-up; the kernel had no such primitive |
| `sceAudioOutput2*`, `sceAudioOutputBlocking` | The audio thread runs at priority 0x10; not blocking starved the other two |
| User partition sized from guest RAM | The stack landed outside RAM on a 32 MiB console |
| Module info without section names | No imports at all, so no HLE at all |

## 1. System calls the game makes that are not implemented

The game imports **228 functions from 25 libraries. 49 of them still have no implementation** and are bound to a logging stub that prints the call once and returns 0. A stub that returns 0 is a lie, and the game acts on it, so these are the first thing to work through.

The rest, by area. What has been implemented has been taken out of these tables, so what is left is what is left.

### Blocking, or likely to be

| Library | Missing | Why it matters |
| --- | --- | --- |
| `ThreadManForUser` (3 of 31) | `sceKernelWaitThreadEnd`, `…EndCB`, `sceKernelWaitSemaCB` | The callback-polling forms of waits that do exist |
| `UtilsForUser` (3 of 7) | `sceKernelDcacheWritebackAll`, `…InvalidateAll`, `…Range` | No-ops on this host, but the game calls them before handing buffers to the GE, so they must at least return |
| `IoFileMgrForUser` (1 of 9) | `sceIoIoctl` | The other half of raw UMD access. Not reached yet |

### Audio

| Library | Missing | Notes |
| --- | --- | --- |
| `sceSasCore` (13 of 27) | `__sceSasSetADSR`, `SetADSRmode`, `SetSL`, `GetEnvelopeHeight`, `GetAllEnvelopeHeights`, `SetGrain`, `GetGrain`, `SetNoise`, `SetOutputmode`, `SetVoicePCM`, `SetPause`, `GetPauseFlag`, `CoreWithMix`, and the four `__sceSasRev*` reverb calls | The mixer's envelope, grain, noise and reverb control. The first port never used them; this game drives all of it |
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
| `vfpu4 not lowered yet` | 390 | 24 distinct instruction words. **`0xD03CA084` is where the port stops today**, at 0x0882EAD4, on one site. `0xD0210000` accounts for 359 sites through 0x08882C00–0x088FB344 and is certain to be next. The rest are two small families, `0xD03Fxxxx` (22 sites) and `0xD05Bxxxx` (8). In the decoder's fields these are group 1: the conversion family, just past the `vuc2i`/`vc2i`/`vus2i`/`vs2i` entries it already has |
| `guest break trap` | 41 | `break` instructions: the game's own assertion traps. Correct as they are |
| `unknown not lowered yet` | 4 | All four are `addi` (opcode 0x08), the trapping add-immediate. The recompiler lowers `addiu` but not this |

The VFPU words need decoding against the hardware reference before anything is implemented; they are recorded here as words, not as guesses at what they do.

## 3. Graphics

**Still not known.** The game now reads its data and runs its threads, but it stops on the VFPU instruction above before submitting a display list. Nothing can be said about what it asks of the GE until it draws, and this section stays empty rather than being filled with guesses.

## 4. Save data

The game writes `ms0:/PSP/SAVEDATA/ULES01456/…` and `ULES01456INST/DATAINST.BIN`. The framework implements the PSP save-data format — the `PARAM.SFO` layout, the encryption of the data file and the hashes that protect it — so saves should be exchangeable with a PSP. **Unverified:** no save has been made or read, the data file's name is not known, and whether this game encrypts its save at all has not been checked.

## 5. Not verified at all

- Anything on Linux or Windows. Everything here is macOS on Apple Silicon.
- Speed. The game's frame loop advances, but it draws nothing, so the numbers mean nothing yet. Guest time ran about 20,000x faster than real time in a headless run, which is what a loop with no work in it and no renderer to throttle it looks like.
- The ad hoc product code in the profile is a guess; the game's own is not known.
- That the game has no code overlays. The profile declares none, on the grounds that nothing has suggested otherwise, which is not the same as having looked.
