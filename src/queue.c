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
	queue_clear(queue);
	return queue;
}

void queue_free(queue_t *queue)
{
	free(queue->data);
	free(queue);
}

void queue_queue(queue_t *queue, void *alloc, void *start, size_t size)
{
	queue_entry_t *free_entry = &queue->data[queue->free_start];

	free_entry->alloc = alloc;
	free_entry->start = start;
	free_entry->size = size;

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

	if (queue->free_start == NOLINK) {
		queue->free_end = NOLINK;
	}

	free_entry->next = NOLINK;
}

size_t queue_find(const queue_t *queue, void *alloc, void *start)
{
	size_t next_index = queue->data_start;
	while (next_index != NOLINK) {
		queue_entry_t *entry = &queue->data[next_index];
		if (entry->alloc == alloc && entry->start == start) {
			return next_index;
		}

		next_index = entry->next;
	}

	return NOLINK;
}

queue_entry_t queue_dequeue(queue_t *queue)
{
	return queue_remove_at(queue, queue->data_start);
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
	entry->prev = queue->free_end;
	entry->next = NOLINK;

	queue->free_end = index;
	if (queue->free_start == NOLINK) {
		queue->free_start = index;
	}

	return *entry;
}

int queue_empty(const queue_t *queue)
{
	return queue->free_start == NOLINK;
}

void queue_reset(queue_t *queue)
{
	queue_clear(queue);
}
