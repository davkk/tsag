# tsag - tickets

From current state (single-file, single-threaded C++ in `src/main.c`) to
the multi-language, multi-threaded pipeline in `docs/ARCHITECTURE.md`
with the single-queue model from `docs/ADR-001-single-queue.md`, plus
incremental updates from the README goals.

## Phase 1 - Multi-language, still single-threaded

- [x] T1: `LanguageCache` - mutex + map keyed by extension, holding
  `DynLib` handle + `*TSLanguage` + `*TSQuery`. Load once per
  language; reads are lock-free on hit.
- [x] T2: Extension -> language mapping for c, cpp, python,
  javascript.
- [x] T3: Per-language query files (`tags-c.scm`, `tags-cpp.scm`,
  `tags-py.scm`, `tags-js.scm`) co-located with the `.so`; retire
  `src/tags.scm`.
- [x] T4: Configurable parser directory via env var
  (`TSAG_GRAMMARS`) with `./parsers/` as default. Remove the
  hardcoded nvim path at `src/main.c:8`.
- [x] T5: Refactor `parse_file` to take the cache; switch parser
  only when the language changes (same-lang fast path, per
  ARCHITECTURE worker lifecycle).
- [x] T6: Loop over `argv[1..]` for multiple inputs; unknown
  extensions are a no-op (cache miss returns null).

## Phase 2 - File discovery

- [ ] T7: `-R <dir>` recursive walk; explicit paths from argv.
  Reuse T6's extension filter so non-source files are skipped
  silently.

## Phase 3 - Multi-threaded workers (per ADR-001)

- [ ] T8: Bounded MPMC `Io.Queue` for the work queue, filled by
  the discovery thread, drained by workers.
- [ ] T9: Worker pool sized to `getCpuCount()`. Each thread owns
  its `TSParser`, `TSQueryCursor`, and a per-thread arena for
  tag string storage.
- [ ] T10: Each worker sorts its `Tag[]` by name before yielding
  (precondition for the k-way merge).
- [ ] T11: Verify the per-thread cache hit path keeps the mutex
  cold in the common case (sanity-check with a quick benchmark
  or `strace`).

## Phase 4 - K-way merge

- [ ] T12: Second `Io.Queue` for sorted `Tag[]` batches from
  workers.
- [ ] T13: Merge stage: min-heap of `(tag, worker_idx)`, pop
  smallest, write to stdout, push next from the same worker;
  drain until all workers hit EOF.
- [ ] T14: Confirm the output is ctags-compatible
  (`name\tfile\t/^line$/;"\tkind`) against a small fixture and
  `diff` against `universal-ctags` on the same input.

## Phase 5 - Polish

- [ ] T15: Dynamic language extensibility - load additional
  grammars from a user directory at runtime, no rebuild.
- [ ] T16: Tests - one fixture per supported language, expected
  tags snapshot, wired into `make test`.
- [ ] T17: CI - fresh-clone `make` builds (submodule init
  included), flake check, basic smoke run on the test fixtures.

## Phase 6 - Stretch (from README)

- [ ] T18: Incremental tags update - mtime-based; merge new
  tags into the existing `tags` file, drop entries for changed
  or removed files.
