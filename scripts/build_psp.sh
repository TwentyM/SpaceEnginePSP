#!/usr/bin/env bash

set -e


ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

BUILD_DIR="$ROOT_DIR/build"

OUTPUT_DIR="/mnt/c/PSPDev/Builds/SpaceEngine"

PPSSPP_EXE="/mnt/c/Program Files/PPSSPP/PPSSPPWindows64.exe"


echo "================================="
echo " SpaceEngine PSP Build"
echo "================================="
echo


mkdir -p "$BUILD_DIR"


echo "[1/6] Compiling assets..."

mkdir -p \
    "$BUILD_DIR/assets/ships"


python3 \
    "$ROOT_DIR/tools/compile_ship.py" \
    "$ROOT_DIR/assets/ships/sidewinder/export/Sidewinder.gltf" \
    "$BUILD_DIR/assets/ships/Sidewinder.pspmesh"

python3 \
    "$ROOT_DIR/tools/compile_texture.py" \
    "$ROOT_DIR/assets/ships/sidewinder/export/Sidewinder_UV_Texture_V1_5.png" \
    "$BUILD_DIR/assets/ships/Sidewinder.psptx"


echo
echo "[2/6] Configuring..."

cd "$BUILD_DIR"

psp-cmake "$ROOT_DIR"


echo
echo "[3/6] Building..."

cmake --build . --parallel


echo
echo "[4/6] Copying EBOOT..."

mkdir -p \
    "$OUTPUT_DIR"

cp \
    "$BUILD_DIR/EBOOT.PBP" \
    "$OUTPUT_DIR/EBOOT.PBP"


echo
echo "[5/6] Copying assets..."

mkdir -p \
    "$OUTPUT_DIR/assets/ships"

cp \
    "$BUILD_DIR/assets/ships/Sidewinder.pspmesh" \
    "$OUTPUT_DIR/assets/ships/Sidewinder.pspmesh"

cp \
    "$BUILD_DIR/assets/ships/Sidewinder.psptx" \
    "$OUTPUT_DIR/assets/ships/Sidewinder.psptx"


echo
echo "[6/6] Restarting PPSSPP..."


if [ ! -f "$PPSSPP_EXE" ]; then

    echo
    echo "================================="
    echo " ERROR"
    echo "================================="
    echo
    echo "PPSSPP executable not found!"
    echo
    echo "Expected location:"
    echo "$PPSSPP_EXE"
    echo

    exit 1
fi


taskkill.exe \
    /IM PPSSPPWindows64.exe \
    /F \
    > /dev/null 2>&1 || true


PPSSPP_WINDOWS_PATH="$(
    wslpath -w "$PPSSPP_EXE"
)"


EBOOT_WINDOWS_PATH="$(
    wslpath -w "$OUTPUT_DIR/EBOOT.PBP"
)"


echo
echo "PPSSPP:"
echo "$PPSSPP_WINDOWS_PATH"

echo
echo "EBOOT:"
echo "$EBOOT_WINDOWS_PATH"

echo
echo "Launching PPSSPP..."


cmd.exe /C start \
    "" \
    "$PPSSPP_WINDOWS_PATH" \
    "$EBOOT_WINDOWS_PATH"


echo
echo "================================="
echo " BUILD SUCCESSFUL"
echo "================================="

echo
echo "EBOOT:"
echo "$EBOOT_WINDOWS_PATH"

echo
echo "Assets:"
echo "$OUTPUT_DIR/assets"

echo
echo "PPSSPP launched."
echo