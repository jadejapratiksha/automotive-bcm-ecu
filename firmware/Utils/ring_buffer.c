#include "Utils/ring_buffer.h"

#include <string.h>

bool RingBuffer_Init(ring_buffer_t *ring_buffer,
                     void *storage,
                     uint16_t capacity,
                     uint16_t item_size)
{
    if ((ring_buffer == NULL) ||
        (storage == NULL) ||
        (capacity == 0U) ||
        (item_size == 0U))
    {
        return false;
    }

    ring_buffer->buffer = (uint8_t *)storage;

    ring_buffer->capacity = capacity;
    ring_buffer->item_size = item_size;

    ring_buffer->head = 0U;
    ring_buffer->tail = 0U;
    ring_buffer->count = 0U;

    return true;
}

bool RingBuffer_Push(ring_buffer_t *ring_buffer,
                     const void *item)
{
    uint8_t *destination;

    if ((ring_buffer == NULL) ||
        (item == NULL))
    {
        return false;
    }

    if (RingBuffer_IsFull(ring_buffer) == true)
    {
        return false;
    }

    destination =
        &ring_buffer->buffer[
            ((uint32_t)ring_buffer->head *
             ring_buffer->item_size)];

    memcpy(destination,
           item,
           ring_buffer->item_size);

    ring_buffer->head++;

    if (ring_buffer->head >= ring_buffer->capacity)
    {
        ring_buffer->head = 0U;
    }

    ring_buffer->count++;

    return true;
}

bool RingBuffer_Pop(ring_buffer_t *ring_buffer,
                    void *item)
{
    uint8_t *source;

    if ((ring_buffer == NULL) ||
        (item == NULL))
    {
        return false;
    }

    if (RingBuffer_IsEmpty(ring_buffer) == true)
    {
        return false;
    }

    source =
        &ring_buffer->buffer[
            ((uint32_t)ring_buffer->tail *
             ring_buffer->item_size)];

    memcpy(item,
           source,
           ring_buffer->item_size);

    ring_buffer->tail++;

    if (ring_buffer->tail >= ring_buffer->capacity)
    {
        ring_buffer->tail = 0U;
    }

    ring_buffer->count--;

    return true;
}

bool RingBuffer_Peek(const ring_buffer_t *ring_buffer,
                     void *item)
{
    const uint8_t *source;

    if ((ring_buffer == NULL) ||
        (item == NULL))
    {
        return false;
    }

    if (RingBuffer_IsEmpty(ring_buffer) == true)
    {
        return false;
    }

    source =
        &ring_buffer->buffer[
            ((uint32_t)ring_buffer->tail *
             ring_buffer->item_size)];

    memcpy(item,
           source,
           ring_buffer->item_size);

    return true;
}

void RingBuffer_Clear(ring_buffer_t *ring_buffer)
{
    if (ring_buffer == NULL)
    {
        return;
    }

    ring_buffer->head = 0U;
    ring_buffer->tail = 0U;
    ring_buffer->count = 0U;
}

bool RingBuffer_IsEmpty(const ring_buffer_t *ring_buffer)
{
    if (ring_buffer == NULL)
    {
        return true;
    }

    return (ring_buffer->count == 0U);
}

bool RingBuffer_IsFull(const ring_buffer_t *ring_buffer)
{
    if (ring_buffer == NULL)
    {
        return false;
    }

    return (ring_buffer->count >= ring_buffer->capacity);
}

uint16_t RingBuffer_GetCount(const ring_buffer_t *ring_buffer)
{
    if (ring_buffer == NULL)
    {
        return 0U;
    }

    return ring_buffer->count;
}

uint16_t RingBuffer_GetCapacity(const ring_buffer_t *ring_buffer)
{
    if (ring_buffer == NULL)
    {
        return 0U;
    }

    return ring_buffer->capacity;
}
