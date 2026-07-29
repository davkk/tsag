# ADR-001: Single shared queue with on-demand language loading

## Status

Accepted.

## Context

Files in multiple languages enter a worker pool. Each worker needs a
parser set to the right language. Two approaches:

1. **Load-balanced dispatcher** -- router tracks each worker's language,
   routes same-lang files to matching workers.

2. **Single shared queue** -- all file paths into one `Io.Queue`.
   Workers detect language on pull and switch parser on demand.

## Decision

Single shared queue. No dispatcher, no per-language queues.

## Evidence from Tree-sitter source

### ts_parser_set_language (parser.c:2019)

```
ts_parser_set_language(parser, lang):
  ts_parser_reset(self)
  ts_language_delete(self->lang)     // no-op for native .so
  check ABI version                  // 2 compares
  if wasm: start wasm store          // skipped for native .so
  self->language = ts_language_copy(lang)  // identity for native
```

### ts_parser_reset (parser.c:2078)

```
ts_parser_reset(self):
  external_scanner_destroy(self)     // one free + function call
  wasm_store_reset(self)             // skipped for native .so

  ts_subtree_release(self->old_tree)        // free prev result
  ts_subtree_release(self->finished_tree)   // free internal tree

  reusable_node_clear(&self->reusable_node) // drop incr hint
  ts_lexer_reset(&self->lexer, 0)           // reset position
  ts_stack_clear(self->stack)               // clear parse stack
  ts_parser__set_cached_token(self, ...)    // clear lookahead

  // zero 6 flags:
  accept_count, has_scanner_error, has_error,
  canceled_balancing, parse_options, parse_state
```

### TSLanguage (parser.h:107)

All tables are `const` pointers baked into the `.so` at compile time.
No tables are rebuilt or copied on set_language. The struct is fully
immutable.

### What's actually extra on a language switch

Everything under `ts_parser_reset` is needed between any two parses
regardless of language (free old tree, clear stack, reset lexer).
The only switch-specific costs are:

- `external_scanner_destroy` -- one free + one function pointer call
- ABI version check -- 2 integer compares
- Pointer store -- 1 assignment

## Consequences

- Switching languages between files is negligible -- no measurable
  throughput impact.
- Workers are interchangeable -- no affinity needed.
- New languages: add extension mapping + `.so` path. No dispatch
  changes.
- Trivially scalable: one queue, N workers, N = `getCpuCount()`.
