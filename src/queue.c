#include "queue.h"
#include <stdint.h>
#include <stdlib.h>

static void queue_clear(queue_t *queue)
{
	queue_entry_t *entry;

	queue->data_start = NOLINK;
	queue->data_end = NOLINK;
	queue->free_start = 0;
	queue->free_end = queue->size - 1;

	queue->data[0].prev = NOLINK;
	queue->data[0].next = 1;
	for (size_t i = 1; i < queue->size - 1; ++i) {
		entry = &queue->data[i];
		entry->prev = i - 1;
		entry->next = i + 1;
	}

	queue->data[queue->size - 1].prev = queue->size - 2;
	queue->data[queue->size - 1].next = NOLINK;
}

queue_t *queue_alloc(size_t size)
{
	queue_t *queue = malloc(sizeof(queue_t));
	if (!queue) {
		return NULL;
	}

	queue->data = malloc(size * sizeof(queue_entry_t));
	if (!queue->data) {
		free(queue);
		return NULL;
	}

	queue->size = size;
	queue->actual_size = 0;
	queue_clear(queue);
	return queue;
}

void queue_free(queue_t *queue)
{
	free(queue->data);
	free(queue);
}

static int double_size(queue_t *queue)
{
    queue->size *= 2;
    queue->data = realloc(queue->data, queue->size * sizeof(queue_entry_t));
    if (queue->data == NULL) {
        fprintf(stderr, "Increasing the queue size failed\n");
	return 1;
    };

    /* Append the new space to the free list. */
    queue->data[queue->free_end].next = queue->size / 2 - 1;
    queue->data[queue->size / 2 - 1].prev = queue->free_end;
    queue->data[queue->size / 2 - 1].next = queue->size / 2;
    for (size_t i = queue->size / 2; i < queue->size - 1; ++i) {
        queue->data[i].prev = i - 1;
        queue->data[i].next = i + 1;
    }
    queue->data[queue->size - 1].prev = queue->size - 2;
    queue->data[queue->size - 1].next = NOLINK;
    queue->free_end = queue->size - 1;
    return 0;
}

static queue_entry_t *queue_queue(queue_t *queue)
{
	queue->actual_size++;
	/* + 1 so the free list can never be empty */
	if (queue->actual_size + 1 > queue->size) {
		if (double_size(queue)) {
		    return NULL;
		}
	}

	/* We cannibalise the first free entry */
	queue_entry_t *free_entry = &queue->data[queue->free_start];

	if (free_entry->next != NOLINK) {
		queue->data[free_entry->next].prev = NOLINK;
	}

	if (queue->data_end != NOLINK) {
		queue->data[queue->data_end].next = queue->free_start;
	}
	if (queue->data_start == NOLINK) {
		queue->data_start = queue->free_start;
	}

	free_entry->prev = queue->data_end;

	queue->data_end = queue->free_start;
	queue->free_start = free_entry->next;

	free_entry->next = NOLINK;
	return free_entry;

}

int queue_queue_prefetch(queue_t *queue, void *alloc, uintptr_t start, uintptr_t end, gasnet_handle_t handle, uintptr_t prefetch_start)
{
	queue_entry_t *entry = queue_queue(queue);
	if (!entry) {
	    return 1;
	}

	entry->prefetch.alloc = alloc;
	entry->prefetch.start = start;
	entry->prefetch.end = end;
	entry->prefetch.handle = handle;
	entry->prefetch.gottem = 0;
	entry->prefetch.prefetch_start = prefetch_start;
	return 0;
}

queue_entry_t *queue_find_prefetch(const queue_t *queue, uintptr_t address)
{
	size_t next_index = queue->data_start;
	while (next_index != NOLINK) {
		queue_entry_t *entry = &queue->data[next_index];
		if (entry->prefetch.start <= address && address < entry->prefetch.end) {
			return entry;
		}

		next_index = entry->next;
	}

	return NULL;
}

queue_entry_t queue_remove_at(queue_t *queue, size_t index)
{
	queue_entry_t *entry = &queue->data[index];

	/* Update the data list. */
	if (entry->prev != NOLINK) {
		queue->data[entry->prev].next = entry->next;
	}
	if (entry->next != NOLINK) {
		queue->data[entry->next].prev = entry->prev;
	}

	if (index == queue->data_start) {
		queue->data_start = entry->next;
	}
	if (index == queue->data_end) {
		queue->data_end = entry->prev;
	}

	/* Update the free list. */
	if (queue->free_end != NOLINK) {
		queue->data[queue->free_end].next = index;
	}

	entry->prev = queue->free_end;
	entry->next = NOLINK;

	queue->free_end = index;
	if (queue->free_start == NOLINK) {
		queue->free_start = index;
	}

	return *entry;
}

void queue_reset(queue_t *queue)
{
	queue_clear(queue);
}
