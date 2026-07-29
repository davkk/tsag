# Architecture

## Pipeline overview

```

  +----------------+    +--------------+    +------------------+    +----------+
  |  File          |    |  Work Queue  |    |  Worker Pool     |    |  Merge   |
  |  Discovery     |--->|  (Io.Queue)  |--->|  (N = cores)     |--->|  (k-way) |
  |                |    |              |    |                  |    |          |
  | argv[1..]      |    |  bounded buf |    |  per thread:     |    |  N -> 1  |
  | or -R walk     |    |  MPMC        |    |  parser          |    |  sorted  |
  |                |    |  blocking    |    |  cursor          |    |  stdout  |
  +----------------+    |  put/get     |    |  arena           |    +----------+
                        +--------------+    |  lang cache      |
                                            +------------------+
```

## Worker lifecycle

```

  +----------------------------------------------------------------+
  |  loop:                                                         |
  |    filepath = work_q.getOne(io)                                |
  |    lang = extToLang(ext)                                       |
  |      |-- null --+ continue                                     |
  |      |                                                         |
  |      +-- same as last file?                                    |
  |      |     yes --+ skip setup, use existing parser/query       |
  |      |     no  --+ entry = cache.getOrLoad(lang)               |
  |      |           + set_language(entry.ts_lang)                 |
  |      |           + last = lang, last_entry = entry             |
  |      |                                                         |
  |      +-- read file                                             |
  |      +-- parse tree                                            |
  |      +-- query --+ Tag[]                                       |
  |      +-- free tree + reset arena                               |
  |      +-- back to loop                                          |
  |                                                                |
  |  queue closed:                                                 |
  |    sort Tag[] --+ merge_q.putOne(io, Tag[])                    |
  +----------------------------------------------------------------+
```

## Language cache (shared)

```

               +---------------------+
               |  LanguageCache      |
               |                     |
  Worker 1 --->|  mutex              |
  set_language |  map: Lang -> Entry |
  (cpp)        |                     |
               |  Entry {            |
               |    lib: DynLib      |  (loaded once per lang)
               |    ts_lang: *TS     |  (immutable, shared)
               |    query: *Query    |  (immutable, shared)
               |  }                  |
  Worker 2 --->|                     |
  set_language |  .load path:        |
  (python)     |  DynLib.open(       |
               |    "<lang>.so")     |
               |                     |
               +---------------------+

Timeline - each thread loads its own languages independently.
The cache lock is held for microseconds (just the map read/write).

  Time | Thread 1 (cpp)     Thread 2 (cpp)     Thread 3 (python)
  -----+------------------------------------------------------------
   t0  | cpp.so? cache      cpp.so? cache      py.so? cache
       | miss -- lock       hit -- skip        miss -- lock
   t1  | DynLib.open        set_language       DynLib.open
       | tree_sitter_cpp()  parse file         tree_sitter_python()
       | ts_query_new()                        ts_query_new()
       | cache.put(cpp)                        cache.put(python)
       | unlock -- done                        unlock -- done
   t2  | set_language       parse file         set_language
       | parse file         (cont)             parse file
   t3  | parse file         (cont)             parse file
       | (cont)                                (cont)

Both *TSLanguage and *TSQuery are immutable after construction,
safe to read from any thread without synchronization. The mutex
is only hit on cache miss (once per language per thread, not per
file). Threads never block each other's parsing.
```

## Merge stage

```

                         +----------------+
  Worker 1 -- Tag[] ---->|                |
                         |  Merge Queue   |
  Worker 2 -- Tag[] ---->|  (Io.Queue)    |--> k-way heap merge --> stdout
                         |                |
  Worker 3 -- Tag[] ---->|                |
                         +----------------+

  Each Tag[] is locally sorted by name (quicksort).
  Merge thread maintains a min-heap of (tag, worker_idx).
  Pop smallest -> write -> push next tag from same worker.
  Drain all threads -> EOF.
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
  | Language cache      | Yes (mutex)      | rare writes      |
  | Work queue          | Yes (Io.Queue)   | MPMC blocking    |
  | Merge queue         | Yes (Io.Queue)   | MPMC blocking    |
  | Tag arena (worker)  | No (per thread)  | -                |
  | File I/O            | No (per thread)  | -                |
  +---------------------+------------------+------------------+
```
