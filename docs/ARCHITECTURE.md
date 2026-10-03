# Architecture

Current state of the code. Component and symbol names are used for
orientation; no file or line references are kept here on purpose
(they churn too fast to maintain).

## Pipeline overview

```
  +----------------+    +--------------+    +------------------+    +----------+
  |  File          |    |  Work Queue  |    |  Worker Pool     |    |  Merge   |
  |  Discovery     |--->|  (IoQueue)   |--->|  (N = cores-2)   |--->|  (k-way) |
  |                |    |              |    |                  |    |          |
  | argv[1..]      |    |  bounded     |    |  per thread:     |    |  N -> 1  |
  | or "." default |    |  cap = 512   |    |  TSParser        |    |  sorted  |
  | recursive walk |    |  MPMC        |    |  TSQueryCursor   |    |  file    |
  | (no -R flag)   |    |  blocking    |    |  LineVec         |    +----------+
  +----------------+    +--------------+    |  lang cache      |
                                             +------------------+

  threads: 1 main + N workers + 1 merge
```

## Worker lifecycle

```
  +----------------------------------------------------------------+
  |  loop:                                                         |
  |    path = work_q.get()         # blocks; NULL = closed+drained |
  |    ext = after last '.'  ------+-- none ----> skip, free path |
  |    entry = cache.get(ext) -----+-- unknown --> skip, free path |
  |    set_language(entry.lang)     # EVERY file, no fast path    |
  |    src = read whole file       # malloc(n+1); fail/skips: 2MiB cap |
  |    skip generated? ------------+-> long line (>4096) --> skip, no  |
  |                                 |   parse cost at all              |
  |    tree = parse_string(src)                                       |
  |    cursor.exec(entry.query, root)                                 |
  |    for each match (cap 4096/file):                                |
  |      claim(seen, range, pat) --> drop? skip BEFORE any malloc     |
  |      @name + @kind.* --> line "name\tfile\t/^pat$/;\"\tkind\n"  |
  |      (filepath copied; nothing borrowed)                         |
  |      skip: empty / non-printable name, missing kind               |
  |      dedup: same byte range keeps HIGHER pattern_index            |
  |              (bounded per-file table, cap 1024)                   |
  |    free tree + src + path --> back to loop                        |
  |                                                                   |
  |  queue closed:                                                     |
  |    sort LineVec (qsort: strcmp full line) + uniq --> merge_q       |
  +----------------------------------------------------------------+
```

## Tag ownership

```
   LineVec (per worker, freed by merge): char** of fully formatted,
   owned lines. Filepath is copied into each line; kind points into
   the query only transiently during formatting. One retained malloc
   per tag (the line) plus a transient escape buffer.

   no bump arena; no borrowed fields; safe to sort, pass, and spill.
```

## Queues

```
                    put blocks      get blocks      close wakes
                    when FULL       when EMPTY      ALL getters
                       |               |               |
                       v               v               v
  Discovery --[strdup(path)]--> [ Work Queue ] --> worker --> [ Merge Queue ] --> merge
  (main)                        cap = 512        LineVec*      cap = N
                                 paths,           sorted+uniq   (one slot
                                 count not bytes  per worker   per worker)

  get returns NULL only when closed AND drained
```

## Language cache (shared)

```
                 +---------------------+
                 |  LanguageCache      |
                 |  entries[32] fixed  |  (9 langs used)
  Worker 1 --->|  lock               |
  get(ext)     |    hit:  scan,       |
               |      return, unlock   |
  Worker 2 --->|    miss: unlock,     |
  get(ext)     |      dlopen +        |
               |      dlsym +         |
               |      ts_query_new    |  <-- slow path runs WITHOUT
               |      relock, recheck    the lock; concurrent misses
               |      append, unlock     for different languages
                +---------------------+  proceed in parallel

  Entry { name, dl_handle, TSLanguage*, TSQuery* }   # last two immutable,
                                                       shared read-only

  load(lang):  <dir>/<lang>.so --> tree_sitter_<lang> --> embedded
               query lookup --> ts_query_new
               any failure --> NULL --> file skipped

  <dir> = $TSAG_PARSERS or $XDG_DATA_HOME/tsag fallback
```

## Languages and queries

```
  ext --> lang --> embedded query table (no .scm files)
   |
   +-- c                        --> c
   +-- h, cpp, cc, hpp          --> cpp
   +-- py                       --> python
   +-- lua                      --> lua
   +-- js                       --> javascript
   +-- ts                       --> typescript
   +-- zig                      --> zig
   +-- rs                       --> rust
   +-- go                       --> go

  one @name / @kind.* capture set per language, compiled once at first use.
```

## Discovery

```
  enqueue_path(p)
       |
       +-- is_ignored -----------> warn-free drop, subtree pruned
       |                         (substring match on .git, build)
       |
       +-- lstat fails --------> warn, drop
       |
       +-- dir ----------------> opendir/readdir --> recurse each child
       |                         ("." skips "./" prefix; "." / ".." skipped)
       |
       +-- regular file -------> ext prefilter --> unknown ext: drop
       |                      --> lang prefilter --> unknown lang: drop
       |                      --> strdup(p) --> work queue
       |
       +-- else (symlink, ------> ignored
           fifo, socket)
```

## Merge stage

```
                          +----------------+
   Worker 1 -- LineVec --->|                |
                           |  Merge Queue   |
   Worker 2 -- LineVec --->|  (IoQueue)     |--> k-way heap merge --> tags file
                           |                |    pop smallest (strcmp)
   Worker N -- LineVec --->|                |    -> fputs to out
                           +----------------+    -> advance batch -> push
                                |
                    collect ALL batches FIRST,
                    nothing prints before last
                    worker exits
```

## Output format

```
  tags file (or stdout with `-o -`), one line per tag:

  +------+----+------+----+----------------+----+------+
  | name | \t | file | \t | /^pattern$/;" | \t | kind |
  +------+----+------+----+----------------+----+------+
     |          |          |                       |
     |          |          +-- escaped source line  +-- capture suffix
     |          |              of the kind node         after "kind."
     |          +-- copied into the owned line per tag (function, struct,
     +-- copied into the owned line                      method, ...)
```

## Tooling, tests, corpus

```
  corpus/<lang>.* --+-- tsag -- diff --> corpus/expected/<lang>.tags
                    |                   (no make gate currently wires this;
                    |                    compare by hand)
                    |
                    +-- (no compare / parity target exists)

  tsdump ............... parse-tree dump helper
  builds: tsag (-O2) / tsag-debug / tsag-asan / tsdump
  bench:  bench script samples RSS + wall into bench/<stamp>/
```

## Thread-safety

```
  +---------------------+------------------+------------------+
  | Resource            | Shared?          | Why safe         |
  +---------------------+------------------+------------------+
  | TSParser            | No (per thread)  | -                |
  | TSQueryCursor       | No (per thread)  | -                |
  | TSLanguage*         | Yes              | immutable        |
  | TSQuery*            | Yes (read-only)  | immutable        |
  | DynLib handle       | Yes (read-only)  | refcounted by OS |
  | Language cache      | Yes (mutex)      | locked, load once|
  | Work queue          | Yes (IoQueue)    | MPMC blocking    |
  | Merge queue         | Yes (IoQueue)    | MPMC blocking    |
  | LineVec (worker)    | No (per thread)  | -                |
  | Source buf + TSTree | No (per file)    | freed per file   |
  | out file / stdout   | Merge thread only| single writer    |
  +---------------------+------------------+------------------+
```
