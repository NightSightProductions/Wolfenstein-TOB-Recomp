// Wolfenstein: The Old Blood settings for Recomp-Framework (see framework/runtime/game.h).
#include "game.h"

const RtGame rt_game = {
    .name = "Wolfenstein: The Old Blood",
    .exe = "WolfOldBlood_x64.exe",
    .win_dir = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Wolfenstein The Old Blood",
    .data_name = "wolfoldblood-recomp",
    .log_name = "WolfOldBlood-recomp",
    .env_prefix = "WOLF",
    .steam_folder = "Wolfenstein The Old Blood",
    .steam_app_id = 350080,
    .relaunch_prefix = "WolfOldBlood",
    .extra_args = NULL,
    .cloud_save_dir = NULL,
    .cloud_prefix = NULL,
    .computer_name = "MAC",
    .pick_folder_msg = NULL,
    // the DRM-free copy made at build time by tools/unwrap_steamstub.py and bundled into the app
    .image = "WolfOldBlood_x64.exe",
    // id Tech 5 loads glVertexAttrib*ARB (core in 4.1) only when this extension is listed
    .gl_extensions = "GL_ARB_vertex_program",
};
