#ifndef IOQUEUE_H
#define IOQUEUE_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
  void** buf;
  size_t head;
  size_t tail;
  size_t cap;
  size_t count;
  pthread_mutex_t lock;
  pthread_cond_t not_full;
  pthread_cond_t not_empty;
  bool closed;
} IoQueue;

IoQueue* io_queue_new(size_t cap);
void io_queue_free(IoQueue* q);
void io_queue_put(IoQueue* q, void* item);
void* io_queue_get(IoQueue* q);
void io_queue_close(IoQueue* q);

#endif
