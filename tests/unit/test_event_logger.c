#include "unity.h"

#include "event_logger.h"

TEST_SOURCE_FILE("ring_buffer.c")
void setUp(void)
{
    EventLogger_Init();
}


void tearDown(void)
{
}


/*
 * Test 1:
 * Logger should start empty.
 */
void test_EventLogger_ShouldStartEmpty(void)
{
    TEST_ASSERT_EQUAL_UINT16(
        0U,
        EventLogger_GetStoredCount()
    );

    TEST_ASSERT_EQUAL_UINT32(
        0U,
        EventLogger_GetTotalEventCount()
    );
}


/*
 * Test 2:
 * Logging one valid event should increase
 * stored count and total event count.
 */
void test_EventLogger_ShouldStoreOneEvent(void)
{
    EventLogger_Log(
        EVENT_BATTERY_LOW,
        10500U
    );

    TEST_ASSERT_EQUAL_UINT16(
        1U,
        EventLogger_GetStoredCount()
    );

    TEST_ASSERT_EQUAL_UINT32(
        1U,
        EventLogger_GetTotalEventCount()
    );
}


/*
 * Test 3:
 * Retrieved event should match what was logged.
 */
void test_EventLogger_ShouldReturnLoggedEvent(void)
{
    event_record_t record;

    EventLogger_Log(
        EVENT_BATTERY_LOW,
        10500U
    );

    TEST_ASSERT_TRUE(
        EventLogger_GetNext(&record)
    );

    TEST_ASSERT_EQUAL(
        EVENT_BATTERY_LOW,
        record.event_id
    );

    TEST_ASSERT_EQUAL_UINT32(
        10500U,
        record.event_data
    );
}


/*
 * Test 4:
 * GetNext removes the event from storage.
 */
void test_EventLogger_GetNextShouldRemoveEvent(void)
{
    event_record_t record;

    EventLogger_Log(
        EVENT_BATTERY_LOW,
        10500U
    );

    TEST_ASSERT_EQUAL_UINT16(
        1U,
        EventLogger_GetStoredCount()
    );

    TEST_ASSERT_TRUE(
        EventLogger_GetNext(&record)
    );

    TEST_ASSERT_EQUAL_UINT16(
        0U,
        EventLogger_GetStoredCount()
    );
}


/*
 * Test 5:
 * GetNext should return false when no event exists.
 */
void test_EventLogger_GetNextShouldReturnFalseWhenEmpty(void)
{
    event_record_t record;

    TEST_ASSERT_FALSE(
        EventLogger_GetNext(&record)
    );
}


/*
 * Test 6:
 * GetNext should reject NULL pointer.
 */
void test_EventLogger_GetNextShouldRejectNullPointer(void)
{
    TEST_ASSERT_FALSE(
        EventLogger_GetNext(NULL)
    );
}


/*
 * Test 7:
 * Multiple events should be returned FIFO.
 */
void test_EventLogger_ShouldReturnEventsInFIFOOrder(void)
{
    event_record_t record;

    EventLogger_Log(
        EVENT_BATTERY_LOW,
        100U
    );

    EventLogger_Log(
        EVENT_BATTERY_CRITICAL,
        200U
    );

    EventLogger_Log(
        EVENT_BATTERY_OVER_VOLTAGE,
        300U
    );


    TEST_ASSERT_TRUE(
        EventLogger_GetNext(&record)
    );

    TEST_ASSERT_EQUAL(
        EVENT_BATTERY_LOW,
        record.event_id
    );

    TEST_ASSERT_EQUAL_UINT32(
        100U,
        record.event_data
    );


    TEST_ASSERT_TRUE(
        EventLogger_GetNext(&record)
    );

    TEST_ASSERT_EQUAL(
        EVENT_BATTERY_CRITICAL,
        record.event_id
    );

    TEST_ASSERT_EQUAL_UINT32(
        200U,
        record.event_data
    );


    TEST_ASSERT_TRUE(
        EventLogger_GetNext(&record)
    );

    TEST_ASSERT_EQUAL(
        EVENT_BATTERY_OVER_VOLTAGE,
        record.event_id
    );

    TEST_ASSERT_EQUAL_UINT32(
        300U,
        record.event_data
    );
}


/*
 * Test 8:
 * All valid event types should be accepted.
 */
void test_EventLogger_ShouldAcceptAllValidEventTypes(void)
{
    EventLogger_Log(
        EVENT_BATTERY_LOW,
        1U
    );

    EventLogger_Log(
        EVENT_BATTERY_CRITICAL,
        2U
    );

    EventLogger_Log(
        EVENT_BATTERY_OVER_VOLTAGE,
        3U
    );

    EventLogger_Log(
        EVENT_VEHICLE_STATE_CHANGE,
        4U
    );

    TEST_ASSERT_EQUAL_UINT16(
        4U,
        EventLogger_GetStoredCount()
    );

    TEST_ASSERT_EQUAL_UINT32(
        4U,
        EventLogger_GetTotalEventCount()
    );
}


/*
 * Test 9:
 * EVENT_NONE is invalid and must not be logged.
 */
void test_EventLogger_ShouldRejectEventNone(void)
{
    EventLogger_Log(
        EVENT_NONE,
        123U
    );

    TEST_ASSERT_EQUAL_UINT16(
        0U,
        EventLogger_GetStoredCount()
    );

    TEST_ASSERT_EQUAL_UINT32(
        0U,
        EventLogger_GetTotalEventCount()
    );
}


/*
 * Test 10:
 * EVENT_MAX is invalid and must not be logged.
 */
void test_EventLogger_ShouldRejectEventMax(void)
{
    EventLogger_Log(
        EVENT_MAX,
        123U
    );

    TEST_ASSERT_EQUAL_UINT16(
        0U,
        EventLogger_GetStoredCount()
    );

    TEST_ASSERT_EQUAL_UINT32(
        0U,
        EventLogger_GetTotalEventCount()
    );
}


/*
 * Test 11:
 * Value above EVENT_MAX must also be rejected.
 */
void test_EventLogger_ShouldRejectEventAboveMax(void)
{
    event_id_t invalid_event =
        (event_id_t)(EVENT_MAX + 1);

    EventLogger_Log(
        invalid_event,
        123U
    );

    TEST_ASSERT_EQUAL_UINT16(
        0U,
        EventLogger_GetStoredCount()
    );

    TEST_ASSERT_EQUAL_UINT32(
        0U,
        EventLogger_GetTotalEventCount()
    );
}


/*
 * Test 12:
 * Clear should remove all currently stored events.
 */
void test_EventLogger_ClearShouldRemoveStoredEvents(void)
{
    EventLogger_Log(
        EVENT_BATTERY_LOW,
        1U
    );

    EventLogger_Log(
        EVENT_BATTERY_CRITICAL,
        2U
    );

    TEST_ASSERT_EQUAL_UINT16(
        2U,
        EventLogger_GetStoredCount()
    );

    EventLogger_Clear();

    TEST_ASSERT_EQUAL_UINT16(
        0U,
        EventLogger_GetStoredCount()
    );
}


/*
 * Test 13:
 * Clearing stored events must NOT reset the
 * total-event counter.
 */
void test_EventLogger_ClearShouldNotResetTotalEventCount(void)
{
    EventLogger_Log(
        EVENT_BATTERY_LOW,
        1U
    );

    EventLogger_Log(
        EVENT_BATTERY_CRITICAL,
        2U
    );

    EventLogger_Clear();

    TEST_ASSERT_EQUAL_UINT16(
        0U,
        EventLogger_GetStoredCount()
    );

    TEST_ASSERT_EQUAL_UINT32(
        2U,
        EventLogger_GetTotalEventCount()
    );
}


/*
 * Test 14:
 * Reinitialization should reset both storage
 * and total event count.
 */
void test_EventLogger_InitShouldResetLogger(void)
{
    EventLogger_Log(
        EVENT_BATTERY_LOW,
        1U
    );

    EventLogger_Log(
        EVENT_BATTERY_CRITICAL,
        2U
    );

    EventLogger_Init();

    TEST_ASSERT_EQUAL_UINT16(
        0U,
        EventLogger_GetStoredCount()
    );

    TEST_ASSERT_EQUAL_UINT32(
        0U,
        EventLogger_GetTotalEventCount()
    );
}


/*
 * Test 15:
 * Logger capacity is 16 events.
 */
void test_EventLogger_ShouldStoreMaximum16Events(void)
{
    uint32_t i;

    for (i = 0U; i < 16U; i++)
    {
        EventLogger_Log(
            EVENT_BATTERY_LOW,
            i
        );
    }

    TEST_ASSERT_EQUAL_UINT16(
        16U,
        EventLogger_GetStoredCount()
    );

    TEST_ASSERT_EQUAL_UINT32(
        16U,
        EventLogger_GetTotalEventCount()
    );
}


/*
 * Test 16:
 * When a 17th event is added, the oldest
 * event should be discarded.
 */
void test_EventLogger_ShouldDiscardOldestEventWhenFull(void)
{
    uint32_t i;
    event_record_t record;

    /*
     * Fill buffer with data values 0 through 15.
     */
    for (i = 0U; i < 16U; i++)
    {
        EventLogger_Log(
            EVENT_BATTERY_LOW,
            i
        );
    }

    /*
     * Add the 17th event.
     *
     * Oldest event (data = 0) should disappear.
     */
    EventLogger_Log(
        EVENT_BATTERY_CRITICAL,
        16U
    );

    TEST_ASSERT_EQUAL_UINT16(
        16U,
        EventLogger_GetStoredCount()
    );

    TEST_ASSERT_EQUAL_UINT32(
        17U,
        EventLogger_GetTotalEventCount()
    );


    TEST_ASSERT_TRUE(
        EventLogger_GetNext(&record)
    );

    /*
     * Data 0 was discarded.
     * Therefore oldest remaining event is data 1.
     */
    TEST_ASSERT_EQUAL_UINT32(
        1U,
        record.event_data
    );
}


/*
 * Test 17:
 * Total event counter should continue increasing
 * even when the buffer overwrites old events.
 */
void test_EventLogger_TotalCountShouldContinueBeyondBufferCapacity(void)
{
    uint32_t i;

    for (i = 0U; i < 25U; i++)
    {
        EventLogger_Log(
            EVENT_VEHICLE_STATE_CHANGE,
            i
        );
    }

    TEST_ASSERT_EQUAL_UINT16(
        16U,
        EventLogger_GetStoredCount()
    );

    TEST_ASSERT_EQUAL_UINT32(
        25U,
        EventLogger_GetTotalEventCount()
    );
}


/*
 * Test 18:
 * Verify retained data after multiple overwrites.
 *
 * Log values:
 *
 * 0 ... 19
 *
 * Capacity = 16
 *
 * Expected retained:
 *
 * 4 ... 19
 */
void test_EventLogger_ShouldRetainMostRecent16Events(void)
{
    uint32_t i;
    event_record_t record;

    for (i = 0U; i < 20U; i++)
    {
        EventLogger_Log(
            EVENT_VEHICLE_STATE_CHANGE,
            i
        );
    }

    TEST_ASSERT_EQUAL_UINT16(
        16U,
        EventLogger_GetStoredCount()
    );


    for (i = 4U; i < 20U; i++)
    {
        TEST_ASSERT_TRUE(
            EventLogger_GetNext(&record)
        );

        TEST_ASSERT_EQUAL(
            EVENT_VEHICLE_STATE_CHANGE,
            record.event_id
        );

        TEST_ASSERT_EQUAL_UINT32(
            i,
            record.event_data
        );
    }

    TEST_ASSERT_EQUAL_UINT16(
        0U,
        EventLogger_GetStoredCount()
    );
}


/*
 * Test 19:
 * Reading stored events should not decrease the
 * total number of events ever logged.
 */
void test_EventLogger_GetNextShouldNotDecreaseTotalEventCount(void)
{
    event_record_t record;

    EventLogger_Log(
        EVENT_BATTERY_LOW,
        1U
    );

    EventLogger_Log(
        EVENT_BATTERY_CRITICAL,
        2U
    );

    TEST_ASSERT_EQUAL_UINT32(
        2U,
        EventLogger_GetTotalEventCount()
    );

    EventLogger_GetNext(&record);

    TEST_ASSERT_EQUAL_UINT32(
        2U,
        EventLogger_GetTotalEventCount()
    );

    EventLogger_GetNext(&record);

    TEST_ASSERT_EQUAL_UINT32(
        2U,
        EventLogger_GetTotalEventCount()
    );
}


/*
 * Test 20:
 * Event data must preserve the complete uint32_t value.
 */
void test_EventLogger_ShouldPreserve32BitEventData(void)
{
    event_record_t record;

    EventLogger_Log(
        EVENT_VEHICLE_STATE_CHANGE,
        0x12345678UL
    );

    TEST_ASSERT_TRUE(
        EventLogger_GetNext(&record)
    );

    TEST_ASSERT_EQUAL_UINT32(
        0x12345678UL,
        record.event_data
    );
}


/*
 * Test 21:
 * Verify mixed event types preserve both
 * ID and corresponding data.
 */
void test_EventLogger_ShouldPreserveMixedEventRecords(void)
{
    event_record_t record;

    EventLogger_Log(
        EVENT_BATTERY_LOW,
        10500U
    );

    EventLogger_Log(
        EVENT_BATTERY_CRITICAL,
        8500U
    );

    EventLogger_Log(
        EVENT_BATTERY_OVER_VOLTAGE,
        16000U
    );


    TEST_ASSERT_TRUE(
        EventLogger_GetNext(&record)
    );

    TEST_ASSERT_EQUAL(
        EVENT_BATTERY_LOW,
        record.event_id
    );

    TEST_ASSERT_EQUAL_UINT32(
        10500U,
        record.event_data
    );


    TEST_ASSERT_TRUE(
        EventLogger_GetNext(&record)
    );

    TEST_ASSERT_EQUAL(
        EVENT_BATTERY_CRITICAL,
        record.event_id
    );

    TEST_ASSERT_EQUAL_UINT32(
        8500U,
        record.event_data
    );


    TEST_ASSERT_TRUE(
        EventLogger_GetNext(&record)
    );

    TEST_ASSERT_EQUAL(
        EVENT_BATTERY_OVER_VOLTAGE,
        record.event_id
    );

    TEST_ASSERT_EQUAL_UINT32(
        16000U,
        record.event_data
    );
}
