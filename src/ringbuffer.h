#ifndef RINGBUFFER_GUARD
#define RINGBUFFER_GUARD

#include <stddef.h>

/**
 * Ringbuffer cache entry.
 */
typedef struct
{
	void *alloc;
	void *start;
} cache_entry_t;

/**
 * Ringbuffer.
 */
typedef struct
{
	cache_entry_t *data;
	size_t size;
	size_t start;
	size_t end;
} ringbuffer_t;

/**
 * Allocate a new ringbuffer with the given size.
 */
ringbuffer_t *ringbuffer_alloc(size_t size);

/**
 * Free a ringbuffer.
 */
void ringbuffer_free(ringbuffer_t *ring);

/**
 * Add a new entry to the ringbuffer.
 */
void ringbuffer_add(ringbuffer_t *ring, void *alloc, void *start);

/**
 * Get the first entry in the ringbuffer.
 */
cache_entry_t *ringbuffer_front(const ringbuffer_t *ring);

/**
 * Delete the front entry in the ringbuffer.
 */
void ringbuffer_del(ringbuffer_t *ring);

/**
 * Check if the ringbuffer is empty.
 */
int ringbuffer_empty(const ringbuffer_t *ring);

/**
 * Reset the ringbuffer, removing all entries.
 */
void ringbuffer_reset(ringbuffer_t *ring);

#endif
