# What is missing

What this port does not have yet, most blocking first. Everything here was measured, not guessed: the import lists come from the game's own import table, the recompiler's gaps from the code it generated, and the boot failures from running the game.

Platform for everything below: macOS on Apple Silicon, Vulkan through MoltenVK.

## Where the port is

The game's executable is prepared from the disc image, loaded, and started. `module_start` runs. The port has not yet reached the game's own main thread.

## 1. System calls the game makes that are not implemented

The game imports **228 functions from 25 libraries. 71 of them have no implementation** and are bound to a logging stub that prints the call once and returns 0. A stub that returns 0 is a lie, and the game acts on it, so these are the first thing to work through.

The two that stop the boot were implemented in the framework as part of this work: `sceKernelExtendThreadStack`, which must call the function it is given on a stack of its own — stubbing it meant `module_start` returned without ever creating the game's main thread — and the lightweight mutexes (`sceKernelCreateLwMutex` and the `Kernel_Library` lock/unlock pair).

The rest, by area:

### Blocking, or likely to be

| Library | Missing | Why it matters |
| --- | --- | --- |
| `sceDisplay` (4 of 6) | `sceDisplayWaitVblankStart`, `…StartCB`, `…StartMultiCB`, `sceDisplayGetVcount` | The frame loop. A game that cannot wait for vblank either spins or never advances |
| `ThreadManForUser` (8 of 31) | `sceKernelCreateMbx`, `DeleteMbx`, `SendMbx`, `PollMbx`, `ReceiveMbxCB`; `sceKernelWaitThreadEnd`, `…EndCB`; `sceKernelWaitSemaCB` | Mailboxes are a whole IPC primitive the framework does not have. The `…CB` variants are the callback-polling forms of waits that do exist |
| `UtilsForUser` (3 of 7) | `sceKernelDcacheWritebackAll`, `…InvalidateAll`, `…Range` | No-ops on this host, but the game calls them before handing buffers to the GE, so they must at least return |
| `Kernel_Library` (2 of 9) | `sceKernelMemcpy`, `sceKernelCheckThreadStack` | `sceKernelMemcpy` returning 0 without copying corrupts whatever the game expected to be copied |

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
| `IoFileMgrForUser` (1 of 9) | `sceIoIoctl` | Used for raw disc access on the PSP; what this game asks of it is not known yet |
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

**Not known yet.** The game has not drawn a frame, so nothing can be said about what it asks of the GE that the renderer does not provide. This section will be filled in from a run, not from reading.

## 4. Save data

The game writes `ms0:/PSP/SAVEDATA/ULES01456/…` and `ULES01456INST/DATAINST.BIN`. The framework implements the PSP save-data format — the `PARAM.SFO` layout, the encryption of the data file and the hashes that protect it — so saves should be exchangeable with a PSP. **Unverified:** no save has been made or read, the data file's name is not known, and whether this game encrypts its save at all has not been checked.

## 5. Not verified at all

- Anything on Linux or Windows. Everything here is macOS on Apple Silicon.
- Speed. Nothing has run fast enough or long enough to measure.
- The ad hoc product code in the profile is a guess; the game's own is not known.
- That the game has no code overlays. The profile declares none, on the grounds that nothing has suggested otherwise, which is not the same as having looked.
