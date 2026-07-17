#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

CLANG="${CLANG:-clang}"
OXIDE="${OXIDE:-./build/oxide.exe}"
PYTHON="${PYTHON:-python}"

OUT="hv/build_out"
mkdir -p "$OUT"

echo "[1/5] oxide: hv/src/*.ox -> IR"
cat hv/src/vmx.ox \
    hv/src/vmxops.ox \
    hv/src/vmlaunch.ox \
    hv/src/freestanding_runtime.ox \
    hv/src/kernel.ox > "$OUT/combined.ox"
"$OXIDE" emit --freestanding --target x86_64-elf "$OUT/combined.ox" > "$OUT/kernel.ll"

echo "[2/5] clang: IR -> kernel.o"
"$CLANG" --target=x86_64-elf -ffreestanding -c "$OUT/kernel.ll" -o "$OUT/kernel.o"

echo "[3/5] clang: assemble stub.S + vmcs_syms.S -> boot.o"
"$CLANG" --target=x86_64-elf -ffreestanding -c hv/boot/stub.S -o "$OUT/stub.o"
"$CLANG" --target=x86_64-elf -ffreestanding -c hv/boot/vmcs_syms.S -o "$OUT/syms.o"

echo "[4/5] ld.lld: link -> kernel.elf"
LLD="${LLD:-ld.lld}"
"$LLD" -m elf_x86_64 -T hv/boot/linker.ld \
    --entry=_start \
    "$OUT/stub.o" "$OUT/syms.o" "$OUT/kernel.o" \
    -o "$OUT/kernel.elf"

echo "[5/5] validate multiboot2 header"
"$PYTHON" hv/build/mb2check.py "$OUT/kernel.elf"

echo
echo "OK: $OUT/kernel.elf"
"$CLANG" --target=x86_64-elf -print-file-name=llvm-objcopy >/dev/null 2>&1 || true
