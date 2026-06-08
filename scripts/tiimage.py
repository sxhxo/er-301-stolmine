#!/usr/bin/env python3
"""
tiimage replacement — generates TI AM335x boot image header.

Usage: python3 tiimage.py <load_addr> <boot_mode> <input> <output>

Boot modes: MMCSD (NAND/MMC/SD, little-endian), SPI (byte-swapped)

The header is 8 bytes: image_size (total including header), load_addr.
For SPI mode, both fields and the payload are byte-swapped (big-endian).
"""
import struct
import sys

def main():
    if len(sys.argv) < 5:
        print("Usage: tiimage <load_addr> <boot_mode> <input> <output>")
        sys.exit(1)

    load_addr = int(sys.argv[1], 0)
    boot_mode = sys.argv[2].upper()
    in_path = sys.argv[3]
    out_path = sys.argv[4]

    with open(in_path, "rb") as f:
        data = f.read()

    image_size = len(data)
    is_spi = (boot_mode == "SPI")

    if is_spi:
        # Pad to multiple of 4
        if image_size & 3:
            pad = 4 - (image_size & 3)
            data += b"\x00" * pad
            image_size = len(data)

    hdr_size = 8
    total_size = image_size + hdr_size

    if is_spi:
        # Big-endian (byte-swapped) for SPI
        hdr = struct.pack(">II", total_size, load_addr)
    else:
        # Little-endian for MMCSD/NAND/XIP
        hdr = struct.pack("<II", total_size, load_addr)

    with open(out_path, "wb") as f:
        f.write(hdr)
        if is_spi:
            # Each 4-byte word is byte-swapped
            for i in range(0, image_size, 4):
                word = struct.unpack("<I", data[i:i+4])[0]
                f.write(struct.pack(">I", word))
        else:
            f.write(data)

    print(f"  {in_path} -> {out_path}: total={total_size} load=0x{load_addr:08x} mode={boot_mode}")

if __name__ == "__main__":
    main()
