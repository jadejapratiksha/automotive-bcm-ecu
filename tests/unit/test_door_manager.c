#include "unity.h"

#include "door_manager.h"
#include "mock_gpio_driver.h"


void setUp(void)
{
}


void tearDown(void)
{
}


/*
 * Test 1:
 * Door manager initialization should execute safely.
 *
 * Currently DoorManager_Init() does not perform any hardware action.
 * This test simply verifies that calling it does not fail.
 */
void test_DoorManager_InitShouldCompleteSuccessfully(void)
{
    DoorManager_Init();

    TEST_PASS();
}


/*
 * Test 2:
 * Door switch is active-low.
 *
 * GPIO LOW means the physical door is OPEN.
 */
void test_DoorManager_ShouldReturnOpenWhenGpioIsLow(void)
{
    GPIO_Driver_Read_ExpectAndReturn(
        GPIO_CH_DOOR_FL,
        GPIO_STATE_LOW
    );

    door_state_t state;

    state = DoorManager_GetFrontLeftState();

    TEST_ASSERT_EQUAL(
        DOOR_STATE_OPEN,
        state
    );
}


/*
 * Test 3:
 * GPIO HIGH means the physical door is CLOSED.
 */
void test_DoorManager_ShouldReturnClosedWhenGpioIsHigh(void)
{
    GPIO_Driver_Read_ExpectAndReturn(
        GPIO_CH_DOOR_FL,
        GPIO_STATE_HIGH
    );

    door_state_t state;

    state = DoorManager_GetFrontLeftState();

    TEST_ASSERT_EQUAL(
        DOOR_STATE_CLOSED,
        state
    );
}


/*
 * Test 4:
 * Verify DoorManager reads the correct GPIO channel.
 *
 * CMock automatically verifies that GPIO_Driver_Read()
 * was called exactly once using GPIO_CH_DOOR_FL.
 */
void test_DoorManager_ShouldReadFrontLeftDoorChannel(void)
{
    GPIO_Driver_Read_ExpectAndReturn(
        GPIO_CH_DOOR_FL,
        GPIO_STATE_LOW
    );

    (void)DoorManager_GetFrontLeftState();
}