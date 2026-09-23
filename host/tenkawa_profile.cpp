// Everything PortableKit has to be told about Dragon Ball Z: Tenkaichi Tag
// Team. The framework reads the rest out of the game's own executable.
//
// The numbers here were read off the player's disc, not recalled: the disc id
// and title from PSP_GAME/PARAM.SFO, the hashes from the files themselves, the
// tag from offset 0xD0 of EBOOT.BIN, and the load address and memory size from
// the ELF's program headers.

#include "profile.hpp"

#include "psprecomp/elf32.hpp"

namespace portablekit {
namespace {

// The game keeps two folders on the memory stick. The executable names the
// second one itself, in "ms0:/PSP/SAVEDATA/ULES01456INST/DATAINST.BIN": it is
// the install data the game writes from the disc, several hundred megabytes of
// cache it can rebuild, so an export leaves it out.
constexpr SaveFolder kSaveFolders[] = {
    {"ULES01456", "Game data", true},
    {"ULES01456INST", "Install data", false},
};

} // namespace

const GameProfile &game() {
    static const GameProfile profile{
        .app_name = "TenkawaNative",
        .project_name = "Tenkawa",
        .env_prefix = "TENKAWA",
        .data_organization = "Tenkawa",
        .data_application = "DBZTTT",

        .disc_id = "ULES01456",
        .disc_id_display = "ULES-01456",
        .game_title = "Dragon Ball Z: Tenkaichi Tag Team",
        .executable_path_on_disc = "PSP_GAME/SYSDIR/EBOOT.BIN",
        .param_sfo_path_on_disc = "PSP_GAME/PARAM.SFO",
        // PSP_GAME/SYSDIR/EBOOT.BIN of the European release, and the
        // executable it decrypts to.
        .encrypted_executable_sha256 = "dfdb2b26bbeb741e77fd693e78ffc7c7d50d5fc9542431af37fdfbc364dd6633",
        .executable_sha256 = "d3003adeb57650a7d07ee57a5ed335f268c1bd2ed6a0211bb4a7c8b35ab245cc",
        // The "~PSP" header's tag, and the key the published tag tables give
        // for it. The header layout is the framework's; only this is ours.
        .decryption_tag = 0xD91613F0u,
        .decryption_key = {0xEB, 0xFF, 0x40, 0xD8, 0xB4, 0x1A, 0xE1, 0x66,
                           0x91, 0x3B, 0x8F, 0x64, 0xB6, 0xFC, 0xB7, 0x12},

        // The ELF loads at 0x08804040 and reaches 0x08B66A0C, so the image
        // wants the 32 MiB a PSP-1000 has. This is not the 64 MiB layout the
        // PlayStation 3 release of Monster Hunter Portable 3rd uses.
        .load_base = psprecomp::kDefaultPspUserLoadBase,
        .guest_ram_bytes = 32u * 1024u * 1024u,
        .boot_path = "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN",
        // No code overlays are known yet. The game loads its data from
        // PACKFILE.BIN; whether any of it is code has not been established.
        .overlay_slots = {},

        .save_game_name = "ULES01456",
        .save_folders = kSaveFolders,

        // The code other players see. Nothing else is known about this game's
        // ad hoc play yet; it imports the whole matching library.
        .adhoc_product_code = "ULES01456",
    };
    return profile;
}

} // namespace portablekit
