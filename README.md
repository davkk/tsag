# tsag - a more performant alternative to ctags with use of tree-sitter

## goals

- [x] tree-sitter parsing
- [x] multi-threaded file parsing
- [ ] incremental tags update
- [x] support for multiple languages
- [ ] extensibility via dynamic libraries linking?

## architecture

```
  Discovery (main thread, recursive walk)
    -> Work Queue (IoQueue, cap 256)
    -> Worker Pool, N = cores-2 (TSParser + TSQueryCursor + TagVec each,
       grammars lazy-loaded via shared LangCache)
    -> Merge Queue (cap N) -> k-way heap merge -> ctags on stdout
```

Full pipeline, worker lifecycle, and thread-safety notes live in
`docs/ARCHITECTURE.md`; the single-shared-queue decision is recorded
in `docs/ADR-001-single-queue.md`.
