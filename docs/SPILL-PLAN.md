# Spill plan (implemented 2026-10-03)

Goal: bound per-worker RAM with sorted spill runs + `sort -m`.
Status: implemented. Workers flush full LineVec batches mid-file
(parse checks `bytes >= limit` on each match, guarded to flush only
on a byte-range key change so the post-flush dedup reset is exact);
tails fold in as the last run; merge takes the fast in-memory heap
path when nothing spilled, else streams runs via `sort -m -u`.
`WorkerArg.cache_path` (`XDG_CACHE_HOME/tsag/<project>`) is live.
`$TSAG_SPILL_LIMIT` (bytes) overrides the 8MiB default for tests.

## Steps (paths updated post main.c split)

- `src/tagvec.h` — revive as LineVec (`char**` owned formatted lines +
  len/cap/bytes) with push, `qsort(strcmp)`, local dedup, clear, free.
  Replaces mixed-ownership `Tag` + `(name,file)` sort; full-line `strcmp`
  matches `LC_ALL=C sort -m -u` and is safe to flush/pass across threads.
- `src/parse.c` — add `parse_file_vec` using the same query loop plus
  `line_range`/`escape_pattern`, but `snprintf` the
  `name/filepath/pattern/kind` line into a `malloc`'d buffer and push to
  LineVec. Output bytes must stay identical so benches diff clean.
- `src/parse.c` — explicitly defer per-thread escape-buffer reuse and keep
  one malloc per tag for now to keep the refactor reviewable.
- `src/worker.c` — own a LineVec, check `vec.bytes` against `SPILL_LIMIT`
  (e.g. 8MB) after each file; on exceed, sort + locally dedup, write the
  whole vec as one sorted run file, then clear (free strings, keep
  capacity). Peak per worker stays bounded; small repos never spill.
- `src/worker.c` — `mkstemp` lazily on first overflow as
  `spill.<worker>.<seq>` under `cache_path`. Zero temp files/I/O under
  `LIMIT`; each run individually sorted as required for `sort -m`.
- `src/merge.c` — collect run paths from outq plus in-memory tails and exec
  `sort -m -u -o out run...` with `LC_ALL=C` and `-T <cache dir>`. `-m` is a
  streaming k-way merge over pre-sorted inputs, `-u` the global dedup, `-o`
  the final file; `sort` reads inputs itself (no pipe loop, no `dup2`
  stdin, no `SIGPIPE` ignore).
- `src/main.c` — keep output atomic via `sort -o temp` + rename; handle
  many-run scale (fd exhaustion via `--batch-size`, `ARG_MAX` via
  `--files0-from` or chunked two-level merges).
- `src/merge.c` — keep a single fork for `sort -m` (cheap merge only;
  expensive sort already ran in parallel in workers). Full in-process heap
  merge with no fork stays a future step, not this change.
- `tools/bench.sh` — accept on same neovim rev before/after with
  byte-identical tags, wall flat/down, maxrss bounded, plus a
  `LIMIT=256KB` forced-spill run proving the many-run path.
- `docs/ARCHITECTURE.md` — redraw merge-stage diagram to per-worker sorted
  runs + `sort -m` when this lands.

Source: `refactor.qf` (archived 2026-10-03).
