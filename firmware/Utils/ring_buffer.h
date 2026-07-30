#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    uint8_t *buffer;

    uint16_t capacity;
    uint16_t item_size;

    uint16_t head;
    uint16_t tail;
    uint16_t count;

} ring_buffer_t;

/*
 * Initialize a ring buffer.
 *
 * storage      = memory used to hold the items
 * capacity     = number of items
 * item_size    = size of each item in bytes
 */
bool RingBuffer_Init(ring_buffer_t *ring_buffer,
                     void *storage,
                     uint16_t capacity,
                     uint16_t item_size);

/*
 * Add one item.
 *
 * Returns false when buffer is full.
 */
bool RingBuffer_Push(ring_buffer_t *ring_buffer,
                     const void *item);

/*
 * Remove the oldest item.
 *
 * Returns false when buffer is empty.
 */
bool RingBuffer_Pop(ring_buffer_t *ring_buffer,
                    void *item);

/*
 * Read the oldest item without removing it.
 */
bool RingBuffer_Peek(const ring_buffer_t *ring_buffer,
                     void *item);

/*
 * Remove all elements.
 */
void RingBuffer_Clear(ring_buffer_t *ring_buffer);

bool RingBuffer_IsEmpty(const ring_buffer_t *ring_buffer);

bool RingBuffer_IsFull(const ring_buffer_t *ring_buffer);

uint16_t RingBuffer_GetCount(const ring_buffer_t *ring_buffer);

uint16_t RingBuffer_GetCapacity(const ring_buffer_t *ring_buffer);

#endif /* RING_BUFFER_H */
