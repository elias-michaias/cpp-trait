#!/usr/bin/env bash
# test.sh – compile and run all examples, reporting pass/fail.
#
# t1–t6 are C++23 (exercise deducing-this Mixin method syntax).
# t7 is C++20 (exercises the fallback path: trait mechanism without
#              method syntax -- only qualified free-function calls).

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
  local std="$2"
  local name
  name=$(basename "$src" .cpp)
  local bin="$TMPDIR_LOCAL/$name"

  printf "%-12s " "$name:"

  # Compile (suppress macro-redefinition warnings from trait.hpp internals)
  if ! "$CXX" -std="$std" $CXXFLAGS -Wno-macro-redefined -o "$bin" "$src" 2>"$TMPDIR_LOCAL/$name.err"; then
    echo "FAIL  (compile error, -std=$std)"
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

  echo "PASS  (-std=$std)"
  PASS=$((PASS + 1))
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Compiler : $CXX"
echo "Examples : $SCRIPT_DIR/examples/"
echo "─────────────────────────────────"

for src in "$SCRIPT_DIR"/examples/t[1-6].cpp "$SCRIPT_DIR"/examples/t[9].cpp; do
  run_example "$src" "$STD"
done

# t7 is the C++20 fallback test -- always compiled with -std=c++20
# regardless of $STD so it actually exercises the fallback path.
run_example "$SCRIPT_DIR/examples/t7.cpp" "c++20"

# t8 is the fluent API / Self* returns showcase (C++23)
run_example "$SCRIPT_DIR/examples/t8.cpp" "$STD"

echo "─────────────────────────────────"
echo "Results  : $PASS passed, $FAIL failed"

[ "$FAIL" -eq 0 ]
