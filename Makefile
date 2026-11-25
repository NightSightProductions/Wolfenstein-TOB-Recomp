# Wolfenstein: The Old Blood on Recomp-Framework (framework/, a git submodule): static recompilation to native arm64 macOS.
# Adapted from Wolfenstein-TNO-Recomp (same id Tech 5 engine branch).
#   make gen gen-dll gen-bink   translate WolfOldBlood_x64.exe, superscriptx64.dll and bink2w64.dll
#   make -j8 boot               build the runtime + translated game
#   make app                    "build/Wolfenstein The Old Blood.app"
#   make steam-play             register the app with NightSight Recomp (NSR) (needs NotProton)
#
# orig.exe must be the DRM-free (unwrapped) executable: the Steam release's .text is encrypted by SteamStub.
FW        = framework
PYTHON    = .venv/bin/python
BUILD     = build
GAME_EXE ?= orig.exe
APP_NAME  = WolfensteinOldBlood
APP_FILE  = Wolfenstein The Old Blood
APP_DISPLAY = Wolfenstein: The Old Blood
BUNDLE_ID = com.local.wolfoldblood-recomp
APP_CATEGORY = public.app-category.action-games
APP_FLAGS = --no-vulkan --resource build/WolfOldBlood_x64.unwrapped.exe=WolfOldBlood_x64.exe
GAME_SRC  = src/game.c src/native_wolf.c src/imports_wolf.c src/cloud_wolf.c
NATIVE    = tools/native.txt
PATCHES   = tools/patches.txt
EXTRA_LIBS_FORCE = $(BUILD)/libsuperscriptx64.a $(BUILD)/libbink2w64.a

include $(FW)/recomp.mk

# superscriptx64.dll (the script VM) and bink2w64.dll (RAD Bink 2), rebased into the framework's DLL range
SUPERSCRIPT_DLL ?= orig_superscript.dll
$(eval $(call MODULE_RULES,superscriptx64,$(SUPERSCRIPT_DLL),gen_superscript,--rebase 160000000,))
BINK_DLL ?= orig_bink.dll
$(eval $(call MODULE_RULES,bink2w64,$(BINK_DLL),gen_bink,--rebase 162000000 --no-abi-trim,-DRT_TSO=0))

gen-dll: gen-superscriptx64
gen-bink: gen-bink2w64
.PHONY: gen-dll gen-bink

APP_PATH ?= $(CURDIR)/$(BUILD)/$(APP_FILE).app
# NSR's tool ID ("nsr_proton") contains "proton", which Steam for Mac needs before it syncs the game's Auto-Cloud saves
# (Windows' Saved Games) into the tool's compatdata prefix, where src/cloud_wolf.c points the game's saves.
steam-play:
	python3 $(FW)/tools/nsr.py install
	python3 $(FW)/tools/nsr.py add --appid 350080 --app "$(APP_PATH)" --pass-paths --exe 'WolfOldBlood_x64.exe' \
	    --name "Wolfenstein: The Old Blood"
.PHONY: steam-play
