#!/usr/bin/env python3

import argparse
import struct
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    raise RuntimeError(
        "Pillow is required. Install it with: "
        "sudo apt install python3-pil"
    )


MAGIC = b"SEPSTX01"
VERSION = 1

# Same numeric value as GU_PSM_5650.
FORMAT_5650 = 0

MAX_TEXTURE_SIZE = 512


def is_power_of_two(value):
    return (
        value > 0 and
        (value & (value - 1)) == 0
    )


def compile_texture(
    input_path,
    output_path
):
    input_path = Path(input_path)
    output_path = Path(output_path)

    image = Image.open(
        input_path
    ).convert("RGB")

    original_width, original_height = (
        image.size
    )

    if (
        not is_power_of_two(original_width) or
        not is_power_of_two(original_height)
    ):
        raise RuntimeError(
            "Texture dimensions must be "
            "powers of two. "
            f"Current size: "
            f"{original_width}x"
            f"{original_height}"
        )

    width = original_width
    height = original_height

    # Keep power-of-two dimensions and aspect
    # ratio while bringing oversized textures
    # down to PSP-friendly dimensions.
    while (
        width > MAX_TEXTURE_SIZE or
        height > MAX_TEXTURE_SIZE
    ):
        width //= 2
        height //= 2

    if (
        width != original_width or
        height != original_height
    ):
        image = image.resize(
            (width, height),
            Image.Resampling.LANCZOS
        )

    pixel_data = bytearray()

    for y in range(height):
        for x in range(width):
            r, g, b = image.getpixel(
                (x, y)
            )

                        # PSP GU_PSM_5650 uses the PSP
            # BGR-style memory layout:
            #
            # bits  0..4  = red
            # bits  5..10 = green
            # bits 11..15 = blue

            pixel = (
                ((b >> 3) << 11) |
                ((g >> 2) << 5) |
                (r >> 3)
            )

            pixel_data += struct.pack(
                "<H",
                pixel
            )

    output_path.parent.mkdir(
        parents=True,
        exist_ok=True
    )

    with open(
        output_path,
        "wb"
    ) as output:
        output.write(
            struct.pack(
                "<8sIIIII",
                MAGIC,
                VERSION,
                width,
                height,
                FORMAT_5650,
                len(pixel_data)
            )
        )

        output.write(
            pixel_data
        )

    print()
    print(
        "======================================"
    )
    print(
        " SpaceEngine PSP Texture Compiler"
    )
    print(
        "======================================"
    )
    print()

    print(
        f"Input:  {input_path}"
    )

    print(
        f"Output: {output_path}"
    )

    print()

    print(
        f"Original size: "
        f"{original_width}x"
        f"{original_height}"
    )

    print(
        f"PSP size:      "
        f"{width}x{height}"
    )

    print(
        "Format:        RGB565 / GU_PSM_5650"
    )

    print(
        f"Pixel data:    "
        f"{len(pixel_data)} bytes"
    )

    print(
        f"File size:     "
        f"{output_path.stat().st_size} bytes"
    )

    print()
    print(
        "Compilation successful."
    )
    print()


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Compile PNG into a "
            "PSP-friendly texture."
        )
    )

    parser.add_argument(
        "input"
    )

    parser.add_argument(
        "output"
    )

    args = parser.parse_args()

    compile_texture(
        args.input,
        args.output
    )


if __name__ == "__main__":
    main()