#!/bin/bash
set -e
DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$DIR/../.." && pwd)"
OX="$ROOT/oxide"            # use a posix-built compiler on PATH, or $ROOT/build/oxide

"$OX" rt -o "$DIR/oxide_rt.c"
"$OX" build "$DIR/lib.ox" -o "$DIR/lib.o"
"$OX" build "$DIR/main.ox" -o "$DIR/main.o"

clang -O2 "$DIR/main.o" "$DIR/lib.o" "$DIR/oxide_rt.c" -o "$DIR/demo"
"$DIR/demo"
rc=$?
rm -f "$DIR/lib.o" "$DIR/main.o" "$DIR/oxide_rt.c" "$DIR/demo"
exit $rc
