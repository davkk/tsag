#!/usr/bin/env bash
# Overlay multiple bench runs (samples.csv) in one PNG + peak table.
# Usage: tools/plot_compare.sh [--out compare.png] <benchdir|samples.csv> [...]
#   Each arg is a bench output dir (containing samples.csv) or a CSV directly.
#   Labels default to the bench dir basename; override with LABEL=csv pairs:
#     tools/plot_compare.sh --out bench/compare.png "before=bench/a" "after=bench/b"
# Plotting itself lives in tools/compare.gnuplot; this wrapper only resolves
# dirs/CSVs, prints the peak table, and passes FILES/LABELS/OUT through.
set -euo pipefail

OUT="bench/compare.png"
ARGS=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    --out) OUT="${2:?missing --out value}"; shift 2 ;;
    --out=*) OUT="${1#--out=}"; shift ;;
    -h|--help)
      echo "Usage: $0 [--out compare.png] <benchdir|samples.csv> [...]  (or LABEL=path pairs)" >&2
      exit 0 ;;
    *) ARGS+=("$1"); shift ;;
  esac
done
if [[ ${#ARGS[@]} -lt 1 ]]; then
  echo "error: need at least 1 bench dir/CSV" >&2
  exit 1
fi
command -v gnuplot >/dev/null || { echo "error: gnuplot not found" >&2; exit 1; }

CSVS=()
LABELS=()
for a in "${ARGS[@]}"; do
  label=""
  if [[ "$a" == *"="* && ! -e "$a" ]]; then
    label="${a%%=*}"
    a="${a#*=}"
  fi
  csv="$a"
  [[ -d "$a" ]] && csv="$a/samples.csv"
  if [[ ! -f "$csv" ]]; then
    echo "error: no samples.csv in $a" >&2
    exit 1
  fi
  [[ -z "$label" ]] && label="$(basename "$(dirname "$csv")")"
  CSVS+=("$csv")
  LABELS+=("$label")
done

# Peak + sample count per run (awk, no extra deps).
echo "-- peaks --"
printf '%-28s %10s %8s\n' "run" "peak_MB" "samples"
for i in "${!CSVS[@]}"; do
  stat_line="$(awk -F, 'NR>1 { if ($3+0 > max) max=$3+0; n++ } END { printf "%.3f %d", max+0, n+0 }' "${CSVS[$i]}")"
  printf '%-28s %10s %8s\n' "${LABELS[$i]}" "${stat_line% *}" "${stat_line#* }"
done

# Hand FILES/LABELS/OUT to the static gnuplot script (space-separated; paths
# with spaces are not supported by gnuplot's word() — rename such dirs first).
mkdir -p "$(dirname "$OUT")"
gnuplot -e "FILES='${CSVS[*]}'; LABELS='${LABELS[*]}'; OUT='$OUT'" \
  "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/compare.gnuplot"
echo "wrote $OUT"
