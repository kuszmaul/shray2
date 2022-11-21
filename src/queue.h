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
	size_t prev;
	size_t next;
} queue_entry_t;

typedef struct
{
	queue_entry_t *data;
	size_t size;
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

queue_entry_t queue_dequeue(queue_t *queue);

queue_entry_t queue_remove_at(queue_t *queue, size_t index);

int queue_empty(const queue_t *queue);

void queue_reset(queue_t *queue);

#endif
