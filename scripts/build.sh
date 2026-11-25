#!/bin/bash
#
# Builds "Wolfenstein The Old Blood.app" from YOUR copy of Wolfenstein: The Old Blood (Windows version, Steam).
# Nothing from the game is stored in this repository; everything is generated locally from your install.
#
# Usage: scripts/build.sh [path/to/Wolfenstein The Old Blood]   (the folder with WolfOldBlood_x64.exe)

set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
die() { echo "error: $*" >&2; exit 1; }
step() { echo "==> [$1/6] $2"; }

find_game() {
    local c
    for c in \
        "$HOME/Library/Application Support/Steam/steamapps/common/Wolfenstein The Old Blood" \
        "$HOME/Library/Application Support/CrossOver/Bottles/"*"/drive_c/Program Files (x86)/Steam/steamapps/common/Wolfenstein The Old Blood"; do
        [[ -f "$c/WolfOldBlood_x64.exe" ]] && { echo "$c"; return; }
    done
}
GAME="${1:-$(find_game)}"
[[ -n "$GAME" ]] || die "the game was not found. Pass the folder that contains WolfOldBlood_x64.exe"

# The Steam release this project is written against
SHA_EXE="ded2ee4f65a5560ef0d716e622fa5146121d45e3736ed013d74964d02fba1cbd"          # as shipped (SteamStub)
SHA_UNWRAPPED="d5ec86343d2d6b091d99970d8f33d4b54c5a699bd9d6c20c1e94a596f5f702e0"    # after tools/unwrap_steamstub.py
SHA_SUPERSCRIPT="1d2333002b072d9216505202e0125bd5ffbcc025eb0aaf86191d5ff144f74743"
SHA_BINK="2cd7762823e6e55039af31ba1c398867fc865dfbd68c4e87c19f5f7913e91281"

step 1 "Checking requirements"
for t in clang make python3 codesign ar openssl; do
    command -v "$t" >/dev/null 2>&1 || die "'$t' was not found. Install Apple's Command Line Tools: xcode-select --install"
done
[[ "$(uname -m)" == "arm64" ]] || die "this build targets Apple Silicon Macs"

step 2 "Verifying your copy of the game"
check() {
    [[ -f "$1" ]] || die "$1 was not found"
    local sha; sha="$(shasum -a 256 "$1" | awk '{print $1}')"
    [[ "$sha" == "$2" ]] || die "$1 is not from the supported Steam release (SHA-256 $sha)"
}
check "$GAME/WolfOldBlood_x64.exe" "$SHA_EXE"
check "$GAME/superscriptx64.dll" "$SHA_SUPERSCRIPT"
check "$GAME/bink2w64.dll" "$SHA_BINK"

step 3 "Preparing the translator"
[[ -x "$ROOT/.venv/bin/python" ]] || python3 -m venv "$ROOT/.venv"
"$ROOT/.venv/bin/python" -m pip install --quiet --disable-pip-version-check "capstone==5.0.6" "lief==0.16.6"

cd "$ROOT"
mkdir -p build
python3 -I tools/unwrap_steamstub.py "$GAME/WolfOldBlood_x64.exe" build/WolfOldBlood_x64.unwrapped.exe
check build/WolfOldBlood_x64.unwrapped.exe "$SHA_UNWRAPPED"
ln -sf build/WolfOldBlood_x64.unwrapped.exe orig.exe
ln -sf "$GAME/superscriptx64.dll" orig_superscript.dll
ln -sf "$GAME/bink2w64.dll" orig_bink.dll

step 4 "Translating the game's code to C"
make -s gen
make -s gen-dll
make -s gen-bink

step 5 "Compiling"
make -s -j"$(sysctl -n hw.ncpu)" boot

step 6 "Packaging the app"
make -s app
echo "Done: build/Wolfenstein The Old Blood.app"
