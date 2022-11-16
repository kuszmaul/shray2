#include "ringbuffer.h"
#include <stdlib.h>

ringbuffer_t *ringbuffer_alloc(size_t size)
{
	ringbuffer_t *ring = malloc(sizeof(ringbuffer_t));
	if (!ring) {
		return NULL;
	}

	ring->data = malloc(size * sizeof(cache_entry_t));
	if (!ring->data) {
		free(ring);
		return NULL;
	}

	ring->start = 0;
	ring->end = 0;
	ring->size = size;
	return ring;
}

void ringbuffer_free(ringbuffer_t *ring)
{
	free(ring->data);
	free(ring);
}

void ringbuffer_add(ringbuffer_t *ring, void *alloc, void *start)
{
	cache_entry_t *entry = &ring->data[ring->end++];
	ring->end = (ring->end + 1) % ring->size;

	entry->alloc = alloc;
	entry->start = start;
}

cache_entry_t *ringbuffer_front(const ringbuffer_t *ring)
{
	return &ring->data[ring->start];
}

void ringbuffer_del(ringbuffer_t *ring)
{
	ring->start = (ring->start + 1) % ring->size;
}

int ringbuffer_empty(const ringbuffer_t *ring)
{
	return ring->start == ring->end;
}

void ringbuffer_reset(ringbuffer_t *ring)
{
	ring->start = 0;
	ring->end = 0;
}
