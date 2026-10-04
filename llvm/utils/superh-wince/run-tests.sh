#!/bin/sh
# Builds the programs in tests/ for SH-3 Windows CE and for the host, runs the
# SH-3 build in shemu.py and compares the output with the host build.
#
# usage: run-tests.sh <llvm build or install dir> [test.c ...]

set -e

LLVM=${1:?usage: run-tests.sh <llvm bin dir parent> [tests]}
shift
HERE=$(cd "$(dirname "$0")" && pwd)
WORK=${WORK:-$(mktemp -d)}
PYTHON=${PYTHON:-python3}
HOSTCC=${HOSTCC:-cc}
OPTS=${OPTS:-"-O0 -O1 -O2 -Os"}
TARGET="--target=sh3el-unknown-none-wince"
CFLAGS="$TARGET -ffreestanding -fno-builtin -ffunction-sections -fdata-sections -I$HERE/tests"

BUILTINS="$HERE/../../../compiler-rt/lib/builtins"
RT_SOURCES="adddf3 addsf3 subdf3 subsf3 muldf3 mulsf3 divdf3 divsf3 comparedf2 comparesf2
  extendsfdf2 truncdfsf2 fixdfsi fixsfsi fixdfdi fixsfdi fixunsdfsi fixunssfsi fixunsdfdi
  fixunssfdi floatsidf floatsisf floatdidf floatdisf floatunsidf floatunsisf floatundidf
  floatundisf negdf2 negsf2 divsi3 udivsi3 modsi3 umodsi3 divdi3 udivdi3 moddi3 umoddi3
  udivmoddi4 udivmodsi4 divmodsi4 divmoddi4 muldi3 ashldi3 lshrdi3 ashrdi3 clzsi2 ctzsi2
  popcountsi2 clzdi2 ctzdi2 popcountdi2 cmpdi2 ucmpdi2 negdi2 mulsi3 fp_mode"

mkdir -p "$WORK/rt"
if [ ! -f "$WORK/rt/libbuiltins.a" ]; then
  for name in $RT_SOURCES; do
    if [ -f "$BUILTINS/$name.c" ]; then
      "$LLVM/bin/clang" $TARGET -O2 -ffreestanding -c "$BUILTINS/$name.c" \
        -o "$WORK/rt/$name.o"
    fi
  done
  "$LLVM/bin/llvm-ar" rcs "$WORK/rt/libbuiltins.a" "$WORK"/rt/*.o
fi

if [ $# -eq 0 ]; then
  set -- "$HERE"/tests/t_*.c
fi

failures=0
for test in "$@"; do
  name=$(basename "$test" .c)
  "$HOSTCC" -O1 -w -I"$HERE/tests" "$test" "$HERE/tests/rt_host.c" -o "$WORK/$name.host"
  "$WORK/$name.host" > "$WORK/$name.expected"
  for opt in $OPTS; do
    out="$WORK/$name$opt"
    if ! "$LLVM/bin/clang" $CFLAGS $opt -c "$test" -o "$out.o" 2> "$out.log" ||
       ! "$LLVM/bin/clang" $CFLAGS -O1 -c "$HERE/tests/rt_sh.c" -o "$WORK/rt_sh.o" 2>> "$out.log" ||
       ! "$LLVM/bin/ld.lld" -e main --gc-sections --emit-relocs "$out.o" "$WORK/rt_sh.o" \
           "$WORK/rt/libbuiltins.a" -o "$out.elf" 2>> "$out.log"; then
      echo "FAIL $name $opt (build, see $out.log)"
      failures=$((failures + 1))
      continue
    fi
    if ! "$PYTHON" "$HERE/shemu.py" "$out.elf" > "$out.actual" 2>> "$out.log"; then
      echo "FAIL $name $opt (run, see $out.log)"
      failures=$((failures + 1))
      continue
    fi
    if ! cmp -s "$WORK/$name.expected" "$out.actual"; then
      echo "FAIL $name $opt (output, diff $WORK/$name.expected $out.actual)"
      failures=$((failures + 1))
      continue
    fi
    echo "ok   $name $opt"
  done
done
echo "$failures failures, outputs in $WORK"
[ $failures -eq 0 ]
