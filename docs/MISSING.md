# What is missing

What this port does not have yet, most blocking first. Everything here was measured, not guessed: the import lists come from the game's own import table, the recompiler's gaps from the code it generated, and the boot failures from running the game.

Platform for everything below: macOS on Apple Silicon, Vulkan through MoltenVK.

## Where the port is

**The game draws.** It starts, loads, runs its threads, reads its data off the disc, sets up audio, and puts its own start-up screens on the screen, correctly: the clock-frequency notice ("The clock frequency for the PSP system in use is 222 MHz"), then the memory-stick check ("Checking Memory Stick. Please do not turn off power."). Text, the rounded panel, the gradient and the 2D path all work, and a gamepad is read. Captures are not kept here, because captures made from the game's own assets do not belong in this repository.

It stops on the memory-stick screen and stays there. It is not deadlocked: it runs a steady frame loop, ~25,900 frames in a bounded run, polling the pad and all four utility dialogs every frame, which is what its dialog manager does. It never touches `ms0:` at all — so it is stuck *before* the check it is telling you about.

What it stops on is **a PGD-encrypted file on the disc**, and the evidence is exact:

```
[io] open disc0:/sce_lbn0xec26_size0x4A0 flags=0x40000001 -> 0x00000007
[io] ioctl fd=7 ... cmd=0x04100001 in=16 bytes:
     31 C2 91 BB 31 AC 79 F0 71 35 18 22 21 60 D8 C8 (unhandled, returning 0)
[io] devctl umd1: cmd=0x01F300A5 in=16 out=4 sent: 00 00 00 00 27 EC 00 00 00 00 00 00 13 00 00 00
```

Reading those sectors out of the disc image says what they are: LBA `0xEC21` is an ELF, LBA `0xEC26` begins `\0PGD`, and LBA `0xEC27` is gzip. The file is opened with the encrypted-file flag and the sixteen bytes are its key; the framework implements none of that, so the ioctl returns success and a read would hand the game ciphertext. That is [TeamGDB/PortableKit#17](https://github.com/TeamGDB/PortableKit/issues/17), and it is the whole of what stands between this port and its title screen.

## What the game asks of the GE

Now that it draws, this is answerable, and the port answers it itself: the GE reports every command it ignored at the end of a run. Over ~25,900 frames it ignored **60 distinct commands**, every one of them with a value of zero:

| Commands | Times | 
| --- | --- |
| `0x15`, `0x16`, `0x1c`, `0x20`, `0x25`, `0x26`, `0x28`, `0x38`, `0xe8`, `0xe9` | once per frame |
| `0x24`, `0x27` | twice per frame |
| `0x2c`–`0x33`, `0x36`, `0x37`, `0x50`, `0xa1`–`0xaf`, `0xb9`–`0xc1`, `0xc8`, `0xca`, `0xcc`, `0xd0`, `0xd8`–`0xdd`, `0xe2`–`0xe6` | once or twice in the whole run |

The shape of that is worth reading carefully before implementing anything. The long tail of once-in-the-run commands with a zero value is the game clearing state it never uses, and the per-frame group is a fixed preamble it writes every list. **Nothing here has been shown to affect what is drawn** — the screens it does draw are correct. This list is a starting point for when something draws wrongly, not a list of bugs.

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
| The game-data install dialog | It polled the dialog's status for ever; it now completes (without copying, which is said aloud) |
| The UMD and power callbacks | Registered during start-up |
| Six instructions: `vrndi`, `vi2s`, `vt5650`, `vi2uc`, `vrndf1`, `addi` | Neither the recompiler nor the interpreter could execute them |
| Kernel mailboxes | Created during start-up; the kernel had no such primitive |
| `sceAudioOutput2*`, `sceAudioOutputBlocking` | The audio thread runs at priority 0x10; not blocking starved the other two |
| User partition sized from guest RAM | The stack landed outside RAM on a 32 MiB console |
| Module info without section names | No imports at all, so no HLE at all |

## 1. System calls the game makes that are not implemented

**45 of the 228 imports**, and this is the executable's own answer, not an inference from reading the framework's source: run the port with `TENKAWA_LIST_STUBS=1` and it prints exactly this. Each one is bound to a logging stub that prints the call once and returns 0, and a stub that returns 0 is a lie the game acts on.

| Library | Missing | Notes |
| --- | --- | --- |
| `sceNetAdhocMatching` (11 of 11) | `Init`, `Term`, `Create`, `Delete`, `Start`, `Stop`, `SelectTarget`, `CancelTargetWithOpt`, `SetHelloOpt`, `SendData`, `AbortSendData` | The whole peer-matching library. The first port's game did its own matchmaking over `sceNetAdhocctl` and never touched it. [PortableKit#9](https://github.com/TeamGDB/PortableKit/issues/9) |
| `sceSasCore` (9 of 27) | `__sceSasSetADSR`, `SetADSRmode`, `SetSL`, `SetGrain`, `GetGrain`, `SetNoise`, `SetOutputmode`, `GetAllEnvelopeHeights`, `GetPauseFlag` | The mixer's envelope, grain and noise control. The game calls `GetAllEnvelopeHeights` during start-up. [PortableKit#7](https://github.com/TeamGDB/PortableKit/issues/7) |
| `sceUtility` (6 of 21) | The five `sceUtilityGamedataInstall*` calls, and `sceUtilityGetSystemParamString` | The shell's data-install dialog. [PortableKit#10](https://github.com/TeamGDB/PortableKit/issues/10) |
| `sceAtrac3plus` (4 of 5) | `sceAtracLowLevelInitDecoder`, `sceAtracLowLevelDecode`, `sceAtracGetAtracID`, `sceAtracReinit` | This game feeds the decoder raw frames rather than handing it a file. It calls `sceAtracReinit` during start-up. [PortableKit#8](https://github.com/TeamGDB/PortableKit/issues/8) |
| `sceNetAdhocctl` (4 of 12) | `Connect`, `Join`, `GetState`, `GetPeerInfo` | Joining and creating a group |
| `ThreadManForUser` (3 of 31) | `sceKernelWaitSemaCB`, `sceKernelWaitThreadEnd`, `sceKernelWaitThreadEndCB` | The callback-polling forms of waits that do exist |
| `sceUmdUser` (3 of 5) | `sceUmdRegisterUMDCallBack`, `UnRegister…`, `sceUmdWaitDriveStatCB` | Disc-change callbacks. The game registers one during start-up |
| `sceMpeg` (2 of 23) | `sceMpegAvcDecode`, `sceMpegAvcDecodeStop` | AVC video decode proper |
| `IoFileMgrForUser` (1 of 9) | `sceIoIoctl` | The other half of raw UMD access. Not reached yet |
| `sceImpose` (1 of 2) | `sceImposeSetUMDPopup` | Called during start-up and the game carries on regardless |
| `scePower` (1 of 4) | `scePowerUnregisterCallback` | Same |

Of these, only four are known to be called at all so far: `sceImposeSetUMDPopup`, `sceUmdRegisterUMDCallBack`, `sceAtracReinit` and `__sceSasGetAllEnvelopeHeights`. The rest are what the executable imports, which is not the same as what it uses.

## 2. The recompiler

**The whole executable now lowers.** 8579 function seeds, 488049 code addresses, 153 C++ units, in about two and a half minutes. The only sites the recompiler cannot lower are the game's own 41 `break` instructions, which are its assertion traps and are correct as they are.

It did not start that way. 435 sites came out unsupported, and one of them — `0xD03CA084` at `0x0882EAD4` — is where the port stopped, because the interpreter could not execute it either. They turned out to be six distinct instructions, all now implemented in the framework:

| Instruction | Sites | What it does |
| --- | --- | --- |
| `vrndi` | 359 | A random 32-bit pattern per element |
| `vi2s` | 16 | The top halfword of each element, two per destination word |
| `vt5650` | 8 | Packed 8888 colours to 5650 |
| `vi2uc` | 6 | A quad of integers to four bytes — **the one the port stopped on** |
| `vrndf1` | 1 | A random float in [1, 2) |
| `addi` | 4 | Add immediate. `addiu` was lowered; this was not |

One of those is not faithful and is worth knowing about. The VFPU's random generator keeps its state in the control registers RCX0..RCX7, and the PSP's own sequence is not documented anywhere this project can use, so the framework substitutes its own: deterministic, and explicitly not what hardware produces. This game draws from it 359 times and never calls `vrnds` to seed it, so whatever the framework starts with decides every one of those draws. Expect anything driven by it — scattering, timing jitter, idle animation — to differ from a PSP. It will not fail; it will look different.

## 3. Graphics

Answered above, from the game's own display lists.

## 4. Save data

The game writes `ms0:/PSP/SAVEDATA/ULES01456/…` and `ULES01456INST/DATAINST.BIN`. The framework implements the PSP save-data format — the `PARAM.SFO` layout, the encryption of the data file and the hashes that protect it — so saves should be exchangeable with a PSP. **Unverified:** no save has been made or read, the data file's name is not known, and whether this game encrypts its save at all has not been checked.

## 5. Not verified at all

- Anything on Linux or Windows. Everything here is macOS on Apple Silicon.
- Speed. The game's frame loop advances, but it draws nothing, so the numbers mean nothing yet. Guest time ran about 20,000x faster than real time in a headless run, which is what a loop with no work in it and no renderer to throttle it looks like.
- The ad hoc product code in the profile is a guess; the game's own is not known.
- That the game has no code overlays. The profile declares none, on the grounds that nothing has suggested otherwise, which is not the same as having looked.
