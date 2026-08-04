#!/usr/bin/env bash
# Compare tags produced by universal ctags vs tsag for a given file.
# Usage: compare.sh <file>
set -u

file="$1"

ctags --sort=no --fields=+K -o - "$file" > /tmp/opencode/ctags.out 2>/dev/null
build/tsag "$file" > /tmp/opencode/tsag.out 2>/dev/null

awk -F'\t' '{print $1 "\t" $4}' /tmp/opencode/ctags.out | sort -u > /tmp/opencode/ctags.names
awk -F'\t' '{print $1 "\t" $4}' /tmp/opencode/tsag.out | sort -u > /tmp/opencode/tsag.names

echo "=== ctags ==="
cat /tmp/opencode/ctags.out
echo "=== tsag ==="
cat /tmp/opencode/tsag.out
echo "=== in ctags but NOT tsag ==="
comm -23 /tmp/opencode/ctags.names /tmp/opencode/tsag.names
echo "=== in tsag but NOT ctags ==="
comm -13 /tmp/opencode/ctags.names /tmp/opencode/tsag.names
