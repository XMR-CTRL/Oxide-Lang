#!/bin/bash
set -e
DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$DIR/../.." && pwd)"
OX="${OX:-$ROOT/build/oxide.exe}"
cd "$DIR"

"$OX" build --freestanding --entry kmain kmain.ox -o k.o

echo "=== runtime ox_* symbols (expect none) ==="
if command -v llvm-nm >/dev/null 2>&1; then
  llvm-nm k.o | grep -E "ox_|oxide_main" || echo "  none"
  echo "=== exported functions (expect kmain, fib, entry_check) ==="
  llvm-nm k.o | grep " T "
else
  echo "(llvm-nm not on PATH; object built at $DIR/k.o)"
fi
rm -f k.o
