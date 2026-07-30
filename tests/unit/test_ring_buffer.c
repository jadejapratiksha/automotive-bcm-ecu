#include "unity.h"
#include "ring_buffer.h"

static ring_buffer_t ring_buffer;
static uint8_t storage[4];

void setUp(void)
{
    RingBuffer_Init(&ring_buffer,
                    storage,
                    4U,
                    sizeof(uint8_t));
}

void tearDown(void)
{
}

/*
 * Test 1:
 * Buffer should be empty immediately after initialization.
 */
void test_RingBuffer_ShouldBeEmptyAfterInitialization(void)
{
    TEST_ASSERT_TRUE(RingBuffer_IsEmpty(&ring_buffer));
    TEST_ASSERT_FALSE(RingBuffer_IsFull(&ring_buffer));

    TEST_ASSERT_EQUAL_UINT16(0U,
                             RingBuffer_GetCount(&ring_buffer));

    TEST_ASSERT_EQUAL_UINT16(4U,
                             RingBuffer_GetCapacity(&ring_buffer));
}


/*
 * Test 2:
 * Push one item into the buffer.
 */
void test_RingBuffer_ShouldPushOneItem(void)
{
    uint8_t value = 10U;

    TEST_ASSERT_TRUE(
        RingBuffer_Push(&ring_buffer, &value)
    );

    TEST_ASSERT_FALSE(
        RingBuffer_IsEmpty(&ring_buffer)
    );

    TEST_ASSERT_EQUAL_UINT16(
        1U,
        RingBuffer_GetCount(&ring_buffer)
    );
}


/*
 * Test 3:
 * Push and then pop one item.
 */
void test_RingBuffer_ShouldPopOneItem(void)
{
    uint8_t input = 25U;
    uint8_t output = 0U;

    RingBuffer_Push(&ring_buffer, &input);

    TEST_ASSERT_TRUE(
        RingBuffer_Pop(&ring_buffer, &output)
    );

    TEST_ASSERT_EQUAL_UINT8(
        input,
        output
    );

    TEST_ASSERT_TRUE(
        RingBuffer_IsEmpty(&ring_buffer)
    );
}


/*
 * Test 4:
 * Ring buffer must maintain FIFO order.
 */
void test_RingBuffer_ShouldMaintainFIFOOrder(void)
{
    uint8_t value1 = 10U;
    uint8_t value2 = 20U;
    uint8_t value3 = 30U;

    uint8_t output = 0U;

    RingBuffer_Push(&ring_buffer, &value1);
    RingBuffer_Push(&ring_buffer, &value2);
    RingBuffer_Push(&ring_buffer, &value3);

    RingBuffer_Pop(&ring_buffer, &output);
    TEST_ASSERT_EQUAL_UINT8(10U, output);

    RingBuffer_Pop(&ring_buffer, &output);
    TEST_ASSERT_EQUAL_UINT8(20U, output);

    RingBuffer_Pop(&ring_buffer, &output);
    TEST_ASSERT_EQUAL_UINT8(30U, output);

    TEST_ASSERT_TRUE(
        RingBuffer_IsEmpty(&ring_buffer)
    );
}


/*
 * Test 5:
 * Buffer should report full after capacity is reached.
 */
void test_RingBuffer_ShouldReportFull(void)
{
    uint8_t value1 = 1U;
    uint8_t value2 = 2U;
    uint8_t value3 = 3U;
    uint8_t value4 = 4U;

    RingBuffer_Push(&ring_buffer, &value1);
    RingBuffer_Push(&ring_buffer, &value2);
    RingBuffer_Push(&ring_buffer, &value3);
    RingBuffer_Push(&ring_buffer, &value4);

    TEST_ASSERT_TRUE(
        RingBuffer_IsFull(&ring_buffer)
    );

    TEST_ASSERT_EQUAL_UINT16(
        4U,
        RingBuffer_GetCount(&ring_buffer)
    );
}


/*
 * Test 6:
 * Push must fail when buffer is already full.
 */
void test_RingBuffer_ShouldRejectPushWhenFull(void)
{
    uint8_t value1 = 1U;
    uint8_t value2 = 2U;
    uint8_t value3 = 3U;
    uint8_t value4 = 4U;
    uint8_t value5 = 5U;

    RingBuffer_Push(&ring_buffer, &value1);
    RingBuffer_Push(&ring_buffer, &value2);
    RingBuffer_Push(&ring_buffer, &value3);
    RingBuffer_Push(&ring_buffer, &value4);

    TEST_ASSERT_FALSE(
        RingBuffer_Push(&ring_buffer, &value5)
    );

    TEST_ASSERT_EQUAL_UINT16(
        4U,
        RingBuffer_GetCount(&ring_buffer)
    );
}


/*
 * Test 7:
 * Pop must fail when buffer is empty.
 */
void test_RingBuffer_ShouldRejectPopWhenEmpty(void)
{
    uint8_t output = 0U;

    TEST_ASSERT_FALSE(
        RingBuffer_Pop(&ring_buffer, &output)
    );

    TEST_ASSERT_EQUAL_UINT16(
        0U,
        RingBuffer_GetCount(&ring_buffer)
    );
}


/*
 * Test 8:
 * Peek should read oldest item without removing it.
 */
void test_RingBuffer_PeekShouldNotRemoveItem(void)
{
    uint8_t input = 55U;
    uint8_t output = 0U;

    RingBuffer_Push(&ring_buffer, &input);

    TEST_ASSERT_TRUE(
        RingBuffer_Peek(&ring_buffer, &output)
    );

    TEST_ASSERT_EQUAL_UINT8(
        input,
        output
    );

    TEST_ASSERT_EQUAL_UINT16(
        1U,
        RingBuffer_GetCount(&ring_buffer)
    );

    TEST_ASSERT_FALSE(
        RingBuffer_IsEmpty(&ring_buffer)
    );
}


/*
 * Test 9:
 * Peek must fail when buffer is empty.
 */
void test_RingBuffer_PeekShouldFailWhenEmpty(void)
{
    uint8_t output = 0U;

    TEST_ASSERT_FALSE(
        RingBuffer_Peek(&ring_buffer, &output)
    );
}


/*
 * Test 10:
 * Clear should remove all stored items.
 */
void test_RingBuffer_ClearShouldEmptyBuffer(void)
{
    uint8_t value1 = 10U;
    uint8_t value2 = 20U;

    RingBuffer_Push(&ring_buffer, &value1);
    RingBuffer_Push(&ring_buffer, &value2);

    TEST_ASSERT_EQUAL_UINT16(
        2U,
        RingBuffer_GetCount(&ring_buffer)
    );

    RingBuffer_Clear(&ring_buffer);

    TEST_ASSERT_TRUE(
        RingBuffer_IsEmpty(&ring_buffer)
    );

    TEST_ASSERT_EQUAL_UINT16(
        0U,
        RingBuffer_GetCount(&ring_buffer)
    );
}


/*
 * Test 11:
 * Verify wrap-around behavior.
 *
 * Capacity = 4
 *
 * Push:
 * 10 20 30 40
 *
 * Pop:
 * 10 20
 *
 * Push:
 * 50 60
 *
 * Internal head wraps back to beginning of storage.
 *
 * Expected FIFO:
 * 30 40 50 60
 */
void test_RingBuffer_ShouldHandleWrapAround(void)
{
    uint8_t value10 = 10U;
    uint8_t value20 = 20U;
    uint8_t value30 = 30U;
    uint8_t value40 = 40U;
    uint8_t value50 = 50U;
    uint8_t value60 = 60U;

    uint8_t output = 0U;

    RingBuffer_Push(&ring_buffer, &value10);
    RingBuffer_Push(&ring_buffer, &value20);
    RingBuffer_Push(&ring_buffer, &value30);
    RingBuffer_Push(&ring_buffer, &value40);

    RingBuffer_Pop(&ring_buffer, &output);
    TEST_ASSERT_EQUAL_UINT8(10U, output);

    RingBuffer_Pop(&ring_buffer, &output);
    TEST_ASSERT_EQUAL_UINT8(20U, output);

    RingBuffer_Push(&ring_buffer, &value50);
    RingBuffer_Push(&ring_buffer, &value60);

    TEST_ASSERT_TRUE(
        RingBuffer_IsFull(&ring_buffer)
    );

    RingBuffer_Pop(&ring_buffer, &output);
    TEST_ASSERT_EQUAL_UINT8(30U, output);

    RingBuffer_Pop(&ring_buffer, &output);
    TEST_ASSERT_EQUAL_UINT8(40U, output);

    RingBuffer_Pop(&ring_buffer, &output);
    TEST_ASSERT_EQUAL_UINT8(50U, output);

    RingBuffer_Pop(&ring_buffer, &output);
    TEST_ASSERT_EQUAL_UINT8(60U, output);

    TEST_ASSERT_TRUE(
        RingBuffer_IsEmpty(&ring_buffer)
    );
}


/*
 * Test 12:
 * Initialization must reject NULL ring buffer pointer.
 */
void test_RingBuffer_InitShouldRejectNullRingBuffer(void)
{
    uint8_t local_storage[4];

    TEST_ASSERT_FALSE(
        RingBuffer_Init(NULL,
                        local_storage,
                        4U,
                        sizeof(uint8_t))
    );
}


/*
 * Test 13:
 * Initialization must reject NULL storage.
 */
void test_RingBuffer_InitShouldRejectNullStorage(void)
{
    ring_buffer_t local_buffer;

    TEST_ASSERT_FALSE(
        RingBuffer_Init(&local_buffer,
                        NULL,
                        4U,
                        sizeof(uint8_t))
    );
}


/*
 * Test 14:
 * Initialization must reject zero capacity.
 */
void test_RingBuffer_InitShouldRejectZeroCapacity(void)
{
    ring_buffer_t local_buffer;
    uint8_t local_storage[4];

    TEST_ASSERT_FALSE(
        RingBuffer_Init(&local_buffer,
                        local_storage,
                        0U,
                        sizeof(uint8_t))
    );
}


/*
 * Test 15:
 * Initialization must reject zero item size.
 */
void test_RingBuffer_InitShouldRejectZeroItemSize(void)
{
    ring_buffer_t local_buffer;
    uint8_t local_storage[4];

    TEST_ASSERT_FALSE(
        RingBuffer_Init(&local_buffer,
                        local_storage,
                        4U,
                        0U)
    );
}


/*
 * Test 16:
 * Push must reject NULL ring buffer pointer.
 */
void test_RingBuffer_PushShouldRejectNullRingBuffer(void)
{
    uint8_t value = 10U;

    TEST_ASSERT_FALSE(
        RingBuffer_Push(NULL, &value)
    );
}


/*
 * Test 17:
 * Push must reject NULL item pointer.
 */
void test_RingBuffer_PushShouldRejectNullItem(void)
{
    TEST_ASSERT_FALSE(
        RingBuffer_Push(&ring_buffer, NULL)
    );
}


/*
 * Test 18:
 * Pop must reject NULL ring buffer pointer.
 */
void test_RingBuffer_PopShouldRejectNullRingBuffer(void)
{
    uint8_t output = 0U;

    TEST_ASSERT_FALSE(
        RingBuffer_Pop(NULL, &output)
    );
}


/*
 * Test 19:
 * Pop must reject NULL destination pointer.
 */
void test_RingBuffer_PopShouldRejectNullItem(void)
{
    uint8_t value = 10U;

    RingBuffer_Push(&ring_buffer, &value);

    TEST_ASSERT_FALSE(
        RingBuffer_Pop(&ring_buffer, NULL)
    );
}


/*
 * Test 20:
 * Peek must reject NULL ring buffer pointer.
 */
void test_RingBuffer_PeekShouldRejectNullRingBuffer(void)
{
    uint8_t output = 0U;

    TEST_ASSERT_FALSE(
        RingBuffer_Peek(NULL, &output)
    );
}


/*
 * Test 21:
 * Peek must reject NULL destination pointer.
 */
void test_RingBuffer_PeekShouldRejectNullItem(void)
{
    uint8_t value = 10U;

    RingBuffer_Push(&ring_buffer, &value);

    TEST_ASSERT_FALSE(
        RingBuffer_Peek(&ring_buffer, NULL)
    );
}


/*
 * Test 22:
 * Helper functions should safely handle NULL pointers.
 */
void test_RingBuffer_StatusFunctionsShouldHandleNullPointer(void)
{
    TEST_ASSERT_TRUE(
        RingBuffer_IsEmpty(NULL)
    );

    TEST_ASSERT_FALSE(
        RingBuffer_IsFull(NULL)
    );

    TEST_ASSERT_EQUAL_UINT16(
        0U,
        RingBuffer_GetCount(NULL)
    );

    TEST_ASSERT_EQUAL_UINT16(
        0U,
        RingBuffer_GetCapacity(NULL)
    );
}


/*
 * Test 23:
 * Test storage of items larger than one byte.
 */
void test_RingBuffer_ShouldSupportMultiByteItems(void)
{
    ring_buffer_t buffer;

    uint16_t local_storage[3];

    uint16_t input1 = 1000U;
    uint16_t input2 = 2000U;
    uint16_t output = 0U;

    TEST_ASSERT_TRUE(
        RingBuffer_Init(&buffer,
                        local_storage,
                        3U,
                        sizeof(uint16_t))
    );

    TEST_ASSERT_TRUE(
        RingBuffer_Push(&buffer, &input1)
    );

    TEST_ASSERT_TRUE(
        RingBuffer_Push(&buffer, &input2)
    );

    TEST_ASSERT_TRUE(
        RingBuffer_Pop(&buffer, &output)
    );

    TEST_ASSERT_EQUAL_UINT16(
        1000U,
        output
    );

    TEST_ASSERT_TRUE(
        RingBuffer_Pop(&buffer, &output)
    );

    TEST_ASSERT_EQUAL_UINT16(
        2000U,
        output
    );
}