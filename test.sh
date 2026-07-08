#!/usr/bin/env bash
# test.sh – compile and run all examples, reporting pass/fail

set -euo pipefail

CXX=${CXX:-clang}
STD=${STD:-c++23}
CXXFLAGS=${CXXFLAGS:-}
TMPDIR_LOCAL=$(mktemp -d)
trap 'rm -rf "$TMPDIR_LOCAL"' EXIT

PASS=0
FAIL=0

run_example() {
  local src="$1"
  local name
  name=$(basename "$src" .cpp)
  local bin="$TMPDIR_LOCAL/$name"

  printf "%-12s " "$name:"

  # Compile (suppress macro-redefinition warnings from trait.hpp internals)
  if ! "$CXX" -std="$STD" $CXXFLAGS -Wno-macro-redefined -o "$bin" "$src" 2>"$TMPDIR_LOCAL/$name.err"; then
    echo "FAIL  (compile error)"
    echo "      --- stderr ---"
    sed 's/^/      /' "$TMPDIR_LOCAL/$name.err"
    FAIL=$((FAIL + 1))
    return
  fi

  # Run
  if ! "$bin" >"$TMPDIR_LOCAL/$name.out" 2>&1; then
    echo "FAIL  (runtime error)"
    echo "      --- output ---"
    sed 's/^/      /' "$TMPDIR_LOCAL/$name.out"
    FAIL=$((FAIL + 1))
    return
  fi

  echo "PASS"
  PASS=$((PASS + 1))
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Compiler : $CXX -std=$STD"
echo "Examples : $SCRIPT_DIR/examples/"
echo "─────────────────────────────────"

for src in "$SCRIPT_DIR"/examples/t*.cpp; do
  run_example "$src"
done

echo "─────────────────────────────────"
echo "Results  : $PASS passed, $FAIL failed"

[ "$FAIL" -eq 0 ]
