#!/usr/bin/env bash
# Bench peak RSS of tsag on a repo, vmrss-style sampler.
# Usage: tools/bench.sh [repo] [outdir]
#   repo   default: $HOME/code/neovim
#   outdir default: bench/<date>-<basename>-<tsag_rev>/
# Env: RSS_INTERVAL=0.05 BENCH_TIMEOUT=10m TSAG_GRAMMARS=... TSAG_BIN=...
# Output: samples.csv (t_s,rss_mb,total_mb), tags.out, stderr.log,
#         time.log (/usr/bin/time -v ground truth), meta.json + stdout summary.
set -euo pipefail

usage() {
  echo "Usage: $0 [repo] [outdir]" >&2
  echo "Env: RSS_INTERVAL=0.05 BENCH_TIMEOUT=10m TSAG_BIN=build/tsag TSAG_GRAMMARS=..." >&2
}

REPO="${1:-$HOME/code/neovim}"
OUTDIR="${2:-}"

TSAG_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="${TSAG_BIN:-$TSAG_ROOT/build/tsag}"
GRAMMARS="${TSAG_GRAMMARS:-$HOME/.local/share/nvim/site/parser/}"
INTERVAL="${RSS_INTERVAL:-0.05}"
TIMEOUT_DUR="${BENCH_TIMEOUT:-10m}"

if [[ ! -e "$REPO" ]]; then
  echo "error: repo not found: $REPO" >&2
  usage
  exit 1
fi
if [[ ! -x "$BIN" ]]; then
  echo "error: tsag binary not executable: $BIN (run 'make all'?)" >&2
  exit 1
fi
case "$BIN" in
  *asan*|*debug*)
    echo "warning: benching $BIN inflates RSS; prefer build/tsag (-O2)" >&2
    ;;
esac
if [[ ! -d "$GRAMMARS" ]]; then
  echo "warning: grammar dir missing: $GRAMMARS" >&2
fi
export TSAG_GRAMMARS="$GRAMMARS"

tsag_rev="$(git -C "$TSAG_ROOT" rev-parse --short HEAD 2>/dev/null || echo unknown)"
repo_rev="$(git -C "$REPO" rev-parse --short HEAD 2>/dev/null || echo nongit)"
repo_base="$(basename "$REPO")"
stamp="$(date +%F-%H%M%S)"
if [[ -z "$OUTDIR" ]]; then
  OUTDIR="$TSAG_ROOT/bench/${stamp}-${repo_base}-${tsag_rev}"
fi
mkdir -p "$OUTDIR"
CSV="$OUTDIR/samples.csv"
echo "t_s,rss_mb,total_mb" > "$CSV"

# VmRSS (MB) of one pid, 0 if gone. Mirrors vmrss: grep VmRSS /proc/pid/status.
rss_mb_of() {
  local pid="$1" kb
  kb="$(awk '/^VmRSS:/ {print $2; exit}' "/proc/$pid/status" 2>/dev/null || true)"
  [[ -n "${kb:-}" ]] || kb=0
  awk -v k="$kb" 'BEGIN { printf "%.3f", k/1024 }'
}

# Total MB over pid + transitive children (BFS via pgrep -P), like vmrss.
total_mb_of() {
  local root="$1" total="0" p m child
  local -a queue=("$root")
  while [[ ${#queue[@]} -gt 0 ]]; do
    p="${queue[0]}"
    queue=("${queue[@]:1}")
    [[ -d "/proc/$p" ]] || continue
    m="$(rss_mb_of "$p")"
    total="$(awk -v a="$total" -v b="$m" 'BEGIN { printf "%.3f", a+b }')"
    while IFS= read -r child; do
      [[ -n "${child:-}" ]] && queue+=("$child")
    done < <(pgrep -P "$p" 2>/dev/null || true)
  done
  echo "$total"
}

echo "== tsag mem bench =="
echo "repo:     $REPO ($repo_rev)"
echo "bin:      $BIN"
echo "grammars: $GRAMMARS"
echo "out:      $OUTDIR"
echo "interval: ${INTERVAL}s  timeout: $TIMEOUT_DUR"

# Pass 1: sampled run (tsag in background, sample in foreground).
wall_start="$(date +%s.%N)"
# shellcheck disable=SC2086
timeout $TIMEOUT_DUR "$BIN" "$REPO" > "$OUTDIR/tags.out" 2> "$OUTDIR/stderr.log" &
TSAG_PID=$!
# NOTE: $TSAG_PID is the `timeout` wrapper (small RSS); the real tsag is its
# child. Resolve once so rss/total columns track tsag, not the wrapper.
# Without this, rss_mb sits at ~2MB (timeout) and total carries a +2MB bias.
sleep 0.02
SAMPLE_PID="$TSAG_PID"
if tr '\0' ' ' < "/proc/$TSAG_PID/cmdline" 2>/dev/null | grep -q '^timeout '; then
  child_pid="$(pgrep -P "$TSAG_PID" 2>/dev/null | head -1 || true)"
  [[ -n "${child_pid:-}" ]] && SAMPLE_PID="$child_pid"
fi
sampler_start="$EPOCHREALTIME"
samples=0
while kill -0 "$TSAG_PID" 2>/dev/null; do
  # tsag child may have exited just before timeout reaps it; fall back to root.
  pid_to_sample="$SAMPLE_PID"
  if ! kill -0 "$pid_to_sample" 2>/dev/null; then
    pid_to_sample="$TSAG_PID"
    kill -0 "$pid_to_sample" 2>/dev/null || break
  fi
  now="$EPOCHREALTIME"
  t="$(awk -v a="$now" -v b="$sampler_start" 'BEGIN { printf "%.3f", a-b }')"
  rss="$(rss_mb_of "$pid_to_sample")"
  total="$(total_mb_of "$pid_to_sample")"
  printf '%s,%s,%s\n' "$t" "$rss" "$total" >> "$CSV"
  samples=$((samples + 1))
  sleep "$INTERVAL"
done
# Final sample attempt if process exited between kill -0 and read is skipped;
# the last loop row is the peak holder. Do not sample after wait (pid gone).
set +e
wait "$TSAG_PID"
TSAG_RC=$?
set -e
wall_end="$(date +%s.%N)"
wall_s="$(awk -v a="$wall_end" -v b="$wall_start" 'BEGIN { printf "%.3f", a-b }')"

if [[ "$samples" -lt 2 ]]; then
  echo "warning: only $samples sample(s); run finished faster than interval ($INTERVAL s)" >&2
fi

peak_mb="$(awk -F, 'NR>1 { if ($3+0 > max) max=$3+0 } END { printf "%.3f", max+0 }' "$CSV")"
out_bytes="$(wc -c < "$OUTDIR/tags.out" | tr -d ' ')"
out_lines="$(wc -l < "$OUTDIR/tags.out" | tr -d ' ')"

# Pass 2: /usr/bin/time -v ground truth (VmHWM), output discarded.
# shellcheck disable=SC2086
/usr/bin/time -v timeout $TIMEOUT_DUR "$BIN" "$REPO" > /dev/null 2> "$OUTDIR/time.log" || {
  echo "warning: time -v pass failed (see $OUTDIR/time.log)" >&2
}
maxrss_kb="$(grep -i 'Maximum resident set size' "$OUTDIR/time.log" 2>/dev/null | grep -o '[0-9]\+' | head -1 || true)"
[[ -n "${maxrss_kb:-}" ]] || maxrss_kb=0
maxrss_mb="$(awk -v k="$maxrss_kb" 'BEGIN { printf "%.3f", k/1024 }')"

json_escape() {
  local s="$1"
  s="${s//\\/\\\\}"
  s="${s//\"/\\\"}"
  printf '%s' "$s"
}

cat > "$OUTDIR/meta.json" <<EOF
{
  "repo": "$(json_escape "$REPO")",
  "repo_rev": "$(json_escape "$repo_rev")",
  "tsag_rev": "$(json_escape "$tsag_rev")",
  "bin": "$(json_escape "$BIN")",
  "grammars": "$(json_escape "$GRAMMARS")",
  "interval_s": $INTERVAL,
  "wall_s": $wall_s,
  "rc": $TSAG_RC,
  "samples": $samples,
  "peak_total_mb": $peak_mb,
  "maxrss_kb": $maxrss_kb,
  "out_bytes": $out_bytes,
  "out_lines": $out_lines
}
EOF

cat <<EOF
-- summary --
samples:   $samples rows -> $CSV
peak RSS:  $peak_mb MB (sampled total, ${INTERVAL}s tick)
max RSS:   $maxrss_mb MB ($maxrss_kb kB VmHWM via /usr/bin/time -v)
wall:      ${wall_s}s  rc=$TSAG_RC
output:    $out_bytes bytes / $out_lines tags -> $OUTDIR/tags.out
meta:      $OUTDIR/meta.json
EOF
