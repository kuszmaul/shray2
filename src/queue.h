#ifndef QUEUE_GUARD
#define QUEUE_GUARD

#include <stddef.h>
#include <stdint.h>
#include <gasnet.h>

#define NOLINK SIZE_MAX

typedef struct
{
	void *alloc;
	uintptr_t start;
	uintptr_t end;
    gasnet_handle_t handle;
    /* True iff gasnet_wait_syncnb(handle) has been called. */
    int gottem;
	size_t prev;
	size_t next;
} queue_entry_t;

typedef struct
{
	queue_entry_t *data;
    /* We have room for this many elements in the queue. */
	size_t size;
    /* This many elements are actually present in the queue. */
    size_t actualSize;
	size_t data_start;
	size_t data_end;

	size_t free_start;
	size_t free_end;
} queue_t;

queue_t *queue_alloc(size_t size);

void queue_free(queue_t *queue);

void queue_queue(queue_t *queue, void *alloc, uintptr_t start,
        uintptr_t end, gasnet_handle_t handle);

queue_entry_t *queue_find(const queue_t *queue, uintptr_t address);

queue_entry_t queue_remove_at(queue_t *queue, size_t index);

void queue_reset(queue_t *queue);

#endif
