#!/usr/bin/env python3
import struct, sys

MAGIC = 0xE85250D6
SCAN = 32768

def main(path):
    with open(path, "rb") as f:
        data = f.read()
    n = len(data)
    if n < 24:
        print(f"FAIL: file too small ({n} bytes) to hold a multiboot2 header")
        return 1
    search = min(SCAN, n)
    found = None
    for off in range(0, search - 23, 4):
        if data[off] == 0xD6 and data[off+1] == 0x50 and data[off+2] == 0x52 and data[off+3] == 0xE8:
            found = off
            break
    if found is None:
        print(f"FAIL: multiboot2 magic 0xE85250D6 not found in first {search} bytes")
        return 1
    magic, arch, hlen, cksum = struct.unpack_from("<IIII", data, found)
    s = (magic + arch + hlen + cksum) & 0xFFFFFFFF
    print(f"header at file offset 0x{found:x} (within first 32KiB: {found < SCAN})")
    print(f"  magic     = 0x{magic:08X}  (expect 0xE85250D6 -> {'OK' if magic==MAGIC else 'BAD'})")
    print(f"  arch      = 0x{arch:08X}")
    print(f"  length    = {hlen}")
    print(f"  checksum  = 0x{cksum:08X}")
    print(f"  sum mod 2^32 = 0x{s:08X}  (must be 0 -> {'OK' if s==0 else 'BAD'})")
    endoff = found + hlen - 8
    if endoff + 8 <= n:
        etype, eflags, esize = struct.unpack_from("<HHI", data, endoff)
        print(f"  end tag   = type={etype} flags={eflags} size={esize}  "
              f"({'OK' if etype==0 and esize==8 else 'BAD'})")
    if s != 0 or magic != MAGIC or found >= SCAN:
        print("FAIL: header invalid")
        return 1
    print("multiboot2 header: VALID")
    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv[1]))
