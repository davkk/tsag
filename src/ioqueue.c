#include "ioqueue.h"
#include <stdlib.h>

IoQueue* io_queue_new(size_t cap) {
  IoQueue* q = calloc(1, sizeof(IoQueue));
  if (!q) return NULL;
  q->buf = calloc(cap, sizeof(void*));
  if (!q->buf) {
    free(q);
    return NULL;
  }
  q->cap = cap;
  pthread_mutex_init(&q->lock, NULL);
  pthread_cond_init(&q->not_full, NULL);
  pthread_cond_init(&q->not_empty, NULL);
  return q;
}

void io_queue_free(IoQueue* q) {
  if (!q) return;
  free(q->buf);
  pthread_mutex_destroy(&q->lock);
  pthread_cond_destroy(&q->not_full);
  pthread_cond_destroy(&q->not_empty);
  free(q);
}

void io_queue_put(IoQueue* q, void* item) {
  pthread_mutex_lock(&q->lock);
  while (q->count == q->cap) {                 // full?
    pthread_cond_wait(&q->not_full, &q->lock); // sleep, auto-unlock, re-lock on wake
  }

  q->buf[q->tail] = item;
  q->tail = (q->tail + 1) % q->cap;
  q->count++;

  pthread_cond_signal(&q->not_empty); // wake ONE getter
  pthread_mutex_unlock(&q->lock);
}

void* io_queue_get(IoQueue* q) {
  pthread_mutex_lock(&q->lock);
  while (q->count == 0 && !q->closed) {         // empty and not done?
    pthread_cond_wait(&q->not_empty, &q->lock); // sleep
  }

  if (q->count == 0) { // closed + drained
    pthread_mutex_unlock(&q->lock);
    return NULL;
  }

  void* item = q->buf[q->head];
  q->head = (q->head + 1) % q->cap;
  q->count--;

  pthread_cond_signal(&q->not_full); // wake ONE putter
  pthread_mutex_unlock(&q->lock);
  return item;
}

void io_queue_close(IoQueue* q) {
  pthread_mutex_lock(&q->lock);
  q->closed = true;
  pthread_cond_broadcast(&q->not_empty); // wake ALL getters
  pthread_mutex_unlock(&q->lock);
}
