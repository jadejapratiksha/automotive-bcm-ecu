#include "Services/EventLogger/event_logger.h"

#include "Utils/ring_buffer.h"

#include <stddef.h>


#define EVENT_LOG_SIZE    (16U)


/*
 * Physical storage used by the generic ring buffer.
 */
static event_record_t event_storage[EVENT_LOG_SIZE];


/*
 * Ring buffer control structure.
 */
static ring_buffer_t event_ring_buffer;


/*
 * Total number of valid events logged since startup.
 *
 * This counter continues increasing even if older events
 * are overwritten because the buffer becomes full.
 */
static uint32_t total_event_count;


/*
 * Initialize Event Logger.
 */
void EventLogger_Init(void)
{
    total_event_count = 0U;

    (void)RingBuffer_Init(
            &event_ring_buffer,
            event_storage,
            EVENT_LOG_SIZE,
            sizeof(event_record_t));
}


/*
 * Store a new event.
 */
void EventLogger_Log(event_id_t event_id,
                     uint32_t event_data)
{
    event_record_t new_record;

    /*
     * Reject invalid event IDs.
     */
    if ((event_id <= EVENT_NONE) ||
        (event_id >= EVENT_MAX))
    {
        return;
    }

    new_record.event_id = event_id;
    new_record.event_data = event_data;

    /*
     * If the ring buffer is full,
     * discard the oldest event.
     *
     * This allows the logger to always retain
     * the most recent EVENT_LOG_SIZE events.
     */
    if (RingBuffer_IsFull(&event_ring_buffer) == true)
    {
        event_record_t discarded_record;

        (void)RingBuffer_Pop(
                &event_ring_buffer,
                &discarded_record);
    }

    if (RingBuffer_Push(
            &event_ring_buffer,
            &new_record) == true)
    {
        total_event_count++;
    }
}


/*
 * Retrieve the oldest stored event.
 *
 * The event is removed from the buffer.
 */
bool EventLogger_GetNext(event_record_t *record)
{
    if (record == NULL)
    {
        return false;
    }

    return RingBuffer_Pop(
            &event_ring_buffer,
            record);
}


/*
 * Number of events currently stored.
 *
 * Maximum value is EVENT_LOG_SIZE.
 */
uint16_t EventLogger_GetStoredCount(void)
{
    return RingBuffer_GetCount(
            &event_ring_buffer);
}


/*
 * Total number of events logged since initialization.
 */
uint32_t EventLogger_GetTotalEventCount(void)
{
    return total_event_count;
}


/*
 * Clear all currently stored events.
 */
void EventLogger_Clear(void)
{
    RingBuffer_Clear(
            &event_ring_buffer);
}
