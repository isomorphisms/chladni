#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
tmp_dir="$(mktemp -d)"
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM
compiler="${CC:-cc}"
"$compiler" -std=c99 -Wall -Wextra -Werror -pedantic -O2 \
    src/rectangle_modes.c tests/rectangle_modes_test.c -lm -o "$tmp_dir/tests"
"$tmp_dir/tests"
"$compiler" -std=c99 -Wall -Wextra -Werror -pedantic -O2 \
    src/rectangle_modes.c src/main.c -lm -o "$tmp_dir/chladni"
"$tmp_dir/chladni" 1 1 1 2 100 96 antisymmetric > "$tmp_dir/square.pgm"
test "$(head -n 1 "$tmp_dir/square.pgm")" = P5
"$tmp_dir/chladni" 2 1 1 2 100 96 antisymmetric > "$tmp_dir/rejected.pgm" \
    2> "$tmp_dir/error" && { echo 'incorrectly accepted nondegenerate mixture'; exit 1; }
test ! -s "$tmp_dir/rejected.pgm"
echo 'CLI smoke checks passed'
