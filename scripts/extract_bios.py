#!/usr/bin/env python3
import re, tarfile, os, sys

with open("/tmp/bios.bin", "rb") as f:
    data = f.read()

idx = data.rfind(b"\x1f\x8b\x08")
if idx > 0:
    dest = os.path.expanduser("~/ti/bios_6_46_05_55")
    os.makedirs(dest, exist_ok=True)
    payload = "/tmp/bios_payload.tar.gz"
    with open(payload, "wb") as out:
        out.write(data[idx:])
    with tarfile.open(payload, "r:gz") as tar:
        tar.extractall(dest)
    print("Extracted BIOS from payload")
    sys.exit(0)
else:
    print("No gzip payload found")
    sys.exit(1)
