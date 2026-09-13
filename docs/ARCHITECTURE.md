# Architecture

Current state of the code. All `file:line` references are relative
to the repo root.

## Pipeline overview

```
  +----------------+    +--------------+    +------------------+    +----------+
  |  File          |    |  Work Queue  |    |  Worker Pool     |    |  Merge   |
  |  Discovery     |--->|  (IoQueue)   |--->|  (N = cores-2)   |--->|  (k-way) |
  |                |    |              |    |                  |    |          |
  | argv[1..]      |    |  bounded     |    |  per thread:     |    |  N -> 1  |
  | or "." default |    |  cap = 256   |    |  TSParser        |    |  sorted  |
  | recursive walk |    |  MPMC        |    |  TSQueryCursor   |    |  stdout  |
  | (no -R flag)   |    |  blocking    |    |  TagVec          |    +----------+
  +----------------+    +--------------+    |  lang cache      |
                                            +------------------+

  threads: 1 main + N workers + 1 merge          (src/main.c:142-179)
```

## Worker lifecycle

```
  +----------------------------------------------------------------+
  |  loop:                                                         |
  |    path = work_q.get()         # blocks; NULL = closed+drained |
  |    vec.add_path(path)          # ownership -> worker TagVec    |
  |                                # (BEFORE ext check, parse.c:82)|
  |    ext = after last '.'  ------+-- none ----> skip            |
  |                                |   (path retained)             |
  |    entry = cache.get(ext) -----+-- unknown --> skip            |
  |                                |   (path retained)             |
  |    set_language(entry.lang)     # EVERY file, no fast path    |
  |    src = read whole file       # malloc(n+1); fail -> skip    |
  |    tree = parse_string(src)                                       |
  |    cursor.exec(entry.query, root)                                 |
  |    for each match:                                                |
  |      @name + @kind.* --> Tag{ name, file=path, pattern, kind }    |
  |      skip: empty / non-printable name, missing kind               |
  |      dedup: same byte range keeps HIGHER pattern_index            |
  |              (SeenName[1024], parse.c:115-199)                     |
  |    free tree + src --> back to loop                               |
  |                                                                   |
  |  queue closed:                                                     |
  |    sort TagVec (qsort: name, file) --> merge_q.put(vec)            |
  +----------------------------------------------------------------+
```

## Tag ownership

```
   Tag {                          TagVec (per worker, freed by merge)
     name    -- owned ----------> strndup(name)        (parse.c:169)
     file    -- borrowed -------- paths[i]            (parse.c:82,170)
     pattern -- owned ----------> escape_pattern()    (parse.c:165)
     kind    -- borrowed -------- query capture name  ("kind."+5)
   }                               (parse.c:144, src/tagvec.h:9-14)

  no bump arena: one malloc per name + one per pattern, per tag.
```

## Queues

```
                    put blocks      get blocks      close wakes
                    when FULL       when EMPTY      ALL getters
                       |               |               |
                       v               v               v
  Discovery --[strdup(path)]--> [ Work Queue ] --> worker --> [ Merge Queue ] --> merge
  (main)                        cap = 256        TagVec*      cap = N
                                paths,           sorted       (one slot
                                count not bytes  per worker   per worker)
                                (main.c:19)      (main.c:53)  (main.c:151)

  get returns NULL only when closed AND drained  (src/ioqueue.c:42-60)
```

## Language cache (shared)

```
                +---------------------+
                |  LanguageCache      |
                |  entries[10] fixed  |  (src/lang.h:7-21, 9 langs used)
  Worker 1 --->|  lock               |
  get(ext)     |    hit:  scan,       |
               |      return, unlock   |
  Worker 2 --->|    miss: dlopen +    |
  get(ext)     |      dlsym +         |
               |      ts_query_new    |
               |      WHILE LOCKED,   |  <-- misses serialize,
               |      append, unlock     even across languages
                +---------------------+  (src/lang.c:125-151)

  Entry { name, dl_handle, TSLanguage*, TSQuery* }   # last two immutable,
                                                       shared read-only

  load(lang):  <dir>/<lang>.so --> tree_sitter_<lang> --> QUERIES[] lookup
               (src/queries.h) --> ts_query_new  (src/lang.c:26-89)
               any failure --> NULL --> file skipped

  <dir> = $TSAG_GRAMMARS or nvim parser dir fallback  (src/main.c:133-134)
```

## Languages and queries

```
  ext --> lang --> embedded query (src/queries.h QUERIES[], no .scm files)
   |
   +-- c                        --> c
   +-- h, cpp, cc, hpp          --> cpp
   +-- py                       --> python
   +-- lua                      --> lua
   +-- js                       --> javascript
   +-- ts                       --> typescript
   +-- zig                      --> zig
   +-- rs                       --> rust
   +-- go                       --> go          (src/lang.c:10-15)

  one @name / @kind.* capture set per language, compiled once at first use.
```

## Discovery

```
  enqueue_path(p)  (src/main.c:100-130)
       |
       +-- lstat fails --------> warn, drop
       |
       +-- dir ----------------> opendir/readdir --> recurse each child
       |                         ("." skips "./" prefix; "." / ".." skipped)
       |
       +-- regular file -------> strdup(p) --> work queue
       |                         (NO ext prefilter, NO ignore list:
       |                          .git/, build/, bundles all enter here,
       |                          rejected later by ext check, if at all)
       |
       +-- else (symlink, ------> ignored
           fifo, socket)
```

## Merge stage

```
                          +----------------+
   Worker 1 -- TagVec ---->|                |
                           |  Merge Queue   |
   Worker 2 -- TagVec ---->|  (IoQueue)     |--> k-way heap merge --> stdout
                           |                |    (heap.h:12-20)
   Worker N -- TagVec ---->|                |    pop smallest -> printf
                           +----------------+    -> advance batch -> push
                                |
                    collect ALL batches FIRST,
                    nothing prints before last
                    worker exits (main.c:61-98)
```

## Output format

```
  stdout, one line per tag  (src/main.c:86):

  +------+----+------+----+----------------+----+------+
  | name | \t | file | \t | /^pattern$/;" | \t | kind |
  +------+----+------+----+----------------+----+------+
     |          |          |                       |
     |          |          +-- escaped source line  +-- capture suffix
     |          |              of the kind node         after "kind."
     |          +-- borrowed paths[] entry             (function, struct,
     +-- owned strndup                                 method, ...)
```

## Tooling, tests, corpus

```
  corpus/<lang>.* --+-- tsag -- diff --> corpus/expected/<lang>.tags
                    |                   (`make test` gates, `make bless` re-blesses)
                    |
                    +-- compare.sh -- vs -- ctags --sort=no
                         (`make parity`, informational: kind vocabularies
                          differ by design, so it tracks drift, no gate)

  tools/tsdump.c ......... parse-tree dump helper (build/tsdump)
  builds: tsag (-O2) / tsag-debug / tsag-asan / tsdump   (Makefile)
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
  | TagVec (worker)     | No (per thread)  | -                |
  | Source buf + TSTree | No (per file)    | freed per file   |
  | stdout              | Merge thread only| single writer    |
  +---------------------+------------------+------------------+
```
