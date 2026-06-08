#!/usr/bin/env python3
import sys

with open("/tmp/bios.bin", "rb") as f:
    data = f.read()

# Search for gzip magic at the very end
idx = max(data.rfind(b"\x1f\x8b\x08"), data.rfind(b"\x1f\x8b\x00"))
print(idx)
