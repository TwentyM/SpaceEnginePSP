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

cd "$BUILD_DIR"

echo "[1/4] Configuring..."
psp-cmake ..

echo
echo "[2/4] Building..."
cmake --build . --parallel

echo
echo "[3/4] Copying EBOOT..."

mkdir -p "$OUTPUT_DIR"
cp "$BUILD_DIR/EBOOT.PBP" "$OUTPUT_DIR/EBOOT.PBP"

echo
echo "[4/4] Restarting PPSSPP..."

# Ellenőrizzük, hogy létezik-e a PPSSPP.
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

# Ha fut egy korábbi PPSSPP példány, bezárjuk.
taskkill.exe /IM PPSSPPWindows64.exe /F > /dev/null 2>&1 || true

# Linux/WSL útvonalak átalakítása Windows útvonalakká.
PPSSPP_WINDOWS_PATH="$(wslpath -w "$PPSSPP_EXE")"
EBOOT_WINDOWS_PATH="$(wslpath -w "$OUTPUT_DIR/EBOOT.PBP")"

echo
echo "PPSSPP:"
echo "$PPSSPP_WINDOWS_PATH"

echo
echo "EBOOT:"
echo "$EBOOT_WINDOWS_PATH"

echo
echo "Launching PPSSPP..."

# Windows oldalon, leválasztott folyamatként indítjuk.
cmd.exe /C start "" "$PPSSPP_WINDOWS_PATH" "$EBOOT_WINDOWS_PATH"

echo
echo "================================="
echo " BUILD SUCCESSFUL"
echo "================================="
echo
echo "EBOOT:"
echo "$EBOOT_WINDOWS_PATH"
echo
echo "PPSSPP launched."
echo
