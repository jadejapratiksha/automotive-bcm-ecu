#ifndef EVENT_LOGGER_H
#define EVENT_LOGGER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    EVENT_NONE = 0,

    EVENT_BATTERY_LOW,
    EVENT_BATTERY_CRITICAL,
    EVENT_BATTERY_OVER_VOLTAGE,
    EVENT_VEHICLE_STATE_CHANGE,

    EVENT_MAX
} event_id_t;

typedef struct
{
    event_id_t event_id;
    uint32_t event_data;

} event_record_t;


/*
 * Initialize event logger.
 */
void EventLogger_Init(void);


/*
 * Store a new event.
 *
 * If the event buffer is full,
 * the oldest event is discarded.
 */
void EventLogger_Log(event_id_t event_id,
                     uint32_t event_data);


/*
 * Retrieve and remove the oldest stored event.
 *
 * Returns:
 * true  = event returned
 * false = no event available / invalid pointer
 */
bool EventLogger_GetNext(event_record_t *record);


/*
 * Return number of events currently stored.
 */
uint16_t EventLogger_GetStoredCount(void);


/*
 * Return total number of events logged
 * since initialization.
 */
uint32_t EventLogger_GetTotalEventCount(void);


/*
 * Remove all currently stored events.
 */
void EventLogger_Clear(void);


#endif /* EVENT_LOGGER_H */
