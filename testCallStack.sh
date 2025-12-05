#!/usr/bin/env bash
set -euo pipefail

PERF_DATA="perf.data"
PERF_SCRIPT_OUT="out.perf"

rm -f "$PERF_DATA" "$PERF_SCRIPT_OUT"

echo "Running and profiling: $1 $2 $3"

# perf starts the program and exits only when the program exits
sudo perf record -F 99 -g -- "$1" "$2" "$3"

echo "Perf Done: Converting perf.data to text output..."
sudo perf script >"$PERF_SCRIPT_OUT"

echo "Saved:"
echo " - perf.data"
echo " - $PERF_SCRIPT_OUT"
