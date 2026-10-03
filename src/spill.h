#ifndef SPILL_H
#define SPILL_H

#include <stdbool.h>
#include <stddef.h>

#include "linevec.h"

// Bounded-memory spill: when a worker's LineVec reaches `limit` bytes
// mid-file, the batch is sorted, uniq'd, and flushed to a run file under
// `dir`, keeping peak RAM flat no matter how pathological one file is.
// All runs are individually sorted, so the merge can stream them with
// `sort -m`. Small repos never spill: no temp files, no extra I/O.
typedef struct {
  const char* dir; // borrowed cache dir (XDG_CACHE_HOME/tsag/<project>)
  int worker_id;
  unsigned seq;
  char** runs; // owned run paths, in creation order
  size_t n_runs;
  size_t cap;
  size_t limit; // bytes per run
} Spill;

#define SPILL_DEFAULT_LIMIT (8u * 1024u * 1024u) // 8 MiB per run

// Limit in bytes: SPILL_DEFAULT_LIMIT, or $TSAG_SPILL_LIMIT when set to a
// positive integer (test hook to force the many-run path on small inputs).
size_t spill_default_limit(void);

void spill_init(Spill* s, const char* dir, int worker_id, size_t limit);
// Sort + uniq + write vec as one run file, then clear it (capacity kept)
// and record the path. False on I/O error; vec is left untouched so the
// caller can keep accumulating in memory instead of losing tags.
bool spill_flush(Spill* s, LineVec* vec);
// Sort + uniq vec and write one run file. Returns the owned path, or NULL
// on error (vec untouched). Used for tails that never went through flush.
char* spill_write_run(const char* dir, const char* prefix, LineVec* vec);
// Free the path list (not the files; the merge unlinks those).
void spill_dispose(Spill* s);

#endif
