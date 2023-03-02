#include "ringbuffer.h"
#include <stdlib.h>
#include <stdint.h>

static const size_t NOENTRY = SIZE_MAX;

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

	ring->start = NOENTRY;
	ring->end = NOENTRY;
	ring->size = size;
	ring->entries = 0;
	return ring;
}

void ringbuffer_free(ringbuffer_t *ring)
{
	free(ring->data);
	free(ring);
}

void ringbuffer_add(ringbuffer_t *ring, void *alloc, void *start)
{
	if (ring->end == NOENTRY) {
		ring->end = 0;
		ring->start = 0;
	} else {
		ring->end = (ring->end + 1) % ring->size;
		if (ring->end == ring->start) {
			ring->start = (ring->start + 1) % ring->size;
		}
	}

	cache_entry_t *entry = &ring->data[ring->end];

	entry->alloc = alloc;
	entry->start = start;
	++ring->entries;
}

cache_entry_t *ringbuffer_front(const ringbuffer_t *ring)
{
	return &ring->data[ring->start];
}

void ringbuffer_del(ringbuffer_t *ring)
{
	if (ring->start == ring->end) {
		ring->start = NOENTRY;
		ring->end = NOENTRY;
	} else {
		ring->start = (ring->start + 1) % ring->size;
	}
	--ring->entries;
}

int ringbuffer_empty(const ringbuffer_t *ring)
{
	return ring->start == NOENTRY;
}

int ringbuffer_full(const ringbuffer_t *ring)
{
	return ring->size == ring->entries;
}

void ringbuffer_reset(ringbuffer_t *ring)
{
	ring->start = NOENTRY;
	ring->end = NOENTRY;
	ring->entries = 0;
}
