#include "unity.h"

#include "battery_monitor.h"
#include "mock_adc_driver.h"


static uint16_t mock_battery_voltage_mv;


/*
 * CMock callback for ADC_Driver_ReadBatteryMv().
 *
 * Instead of storing a pointer for CMock to copy later,
 * this callback writes the fake voltage directly into
 * BatteryMonitor's output pointer when the ADC function
 * is actually called.
 */
static bool Mock_ADC_Driver_ReadBatteryMv_Callback(
    uint16_t *battery_mv,
    int cmock_num_calls)
{
    (void)cmock_num_calls;

    if (battery_mv == NULL)
    {
        return false;
    }

    *battery_mv = mock_battery_voltage_mv;

    return true;
}


void setUp(void)
{
    mock_battery_voltage_mv = 0U;

    BatteryMonitor_Init();
}


void tearDown(void)
{
}


/*
 * Configure ADC to return a selected voltage.
 */
static void MockAdcBatteryVoltage(uint16_t voltage_mv)
{
    mock_battery_voltage_mv = voltage_mv;

    ADC_Driver_ReadBatteryMv_StubWithCallback(
        Mock_ADC_Driver_ReadBatteryMv_Callback
    );
}

/*
 * Test 1:
 * Initialization should set battery voltage
 * to the default 12.0 V and state to NORMAL.
 */
void test_BatteryMonitor_ShouldInitializeToNormalState(void)
{
    TEST_ASSERT_EQUAL_UINT16(
        12000U,
        BatteryMonitor_GetVoltageMv()
    );

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_NORMAL,
        BatteryMonitor_GetState()
    );

    TEST_ASSERT_FALSE(
        BatteryMonitor_IsLowVoltage()
    );
}


/*
 * Test 2:
 * UpdateVoltageMv should store the supplied value.
 */
void test_BatteryMonitor_UpdateVoltageShouldStoreValue(void)
{
    BatteryMonitor_UpdateVoltageMv(12500U);

    TEST_ASSERT_EQUAL_UINT16(
        12500U,
        BatteryMonitor_GetVoltageMv()
    );
}


/*
 * Test 3:
 * 12.0 V is in the normal range.
 */
void test_BatteryMonitor_ShouldRemainNormalAt12000mV(void)
{
    MockAdcBatteryVoltage(12000U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_NORMAL,
        BatteryMonitor_GetState()
    );

    TEST_ASSERT_EQUAL_UINT16(
        12000U,
        BatteryMonitor_GetVoltageMv()
    );
}


/*
 * Test 4:
 * Voltage below 11.0 V should enter LOW state.
 */
void test_BatteryMonitor_ShouldEnterLowStateBelow11000mV(void)
{
    MockAdcBatteryVoltage(10500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_LOW,
        BatteryMonitor_GetState()
    );

    TEST_ASSERT_TRUE(
        BatteryMonitor_IsLowVoltage()
    );
}


/*
 * Test 5:
 * Exactly 11.0 V remains NORMAL because
 * production logic checks voltage < 11000.
 */
void test_BatteryMonitor_ShouldRemainNormalAtExactly11000mV(void)
{
    MockAdcBatteryVoltage(11000U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_NORMAL,
        BatteryMonitor_GetState()
    );
}


/*
 * Test 6:
 * Voltage below 9.0 V should enter CRITICAL state.
 */
void test_BatteryMonitor_ShouldEnterCriticalStateBelow9000mV(void)
{
    MockAdcBatteryVoltage(8500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_CRITICAL,
        BatteryMonitor_GetState()
    );

    TEST_ASSERT_TRUE(
        BatteryMonitor_IsLowVoltage()
    );
}


/*
 * Test 7:
 * Exactly 9.0 V is not CRITICAL because critical
 * entry requires voltage < 9000.
 *
 * Since 9000 < 11000, state should become LOW.
 */
void test_BatteryMonitor_ShouldEnterLowStateAtExactly9000mV(void)
{
    MockAdcBatteryVoltage(9000U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_LOW,
        BatteryMonitor_GetState()
    );
}


/*
 * Test 8:
 * Voltage above 15.5 V should enter OVER_VOLTAGE.
 */
void test_BatteryMonitor_ShouldEnterOverVoltageAbove15500mV(void)
{
    MockAdcBatteryVoltage(16000U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_OVER_VOLTAGE,
        BatteryMonitor_GetState()
    );

    TEST_ASSERT_FALSE(
        BatteryMonitor_IsLowVoltage()
    );
}


/*
 * Test 9:
 * Exactly 15.5 V remains NORMAL because
 * entry requires voltage > 15500.
 */
void test_BatteryMonitor_ShouldRemainNormalAtExactly15500mV(void)
{
    MockAdcBatteryVoltage(15500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_NORMAL,
        BatteryMonitor_GetState()
    );
}


/*
 * Test 10:
 * LOW state should remain LOW below the
 * 11.5 V recovery threshold.
 */
void test_BatteryMonitor_ShouldRemainLowBelow11500mV(void)
{
    MockAdcBatteryVoltage(10500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_LOW,
        BatteryMonitor_GetState()
    );

    MockAdcBatteryVoltage(11200U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_LOW,
        BatteryMonitor_GetState()
    );
}


/*
 * Test 11:
 * LOW state should recover to NORMAL
 * at exactly 11.5 V.
 */
void test_BatteryMonitor_ShouldRecoverFromLowAt11500mV(void)
{
    MockAdcBatteryVoltage(10500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_LOW,
        BatteryMonitor_GetState()
    );

    MockAdcBatteryVoltage(11500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_NORMAL,
        BatteryMonitor_GetState()
    );
}


/*
 * Test 12:
 * LOW state should transition to CRITICAL
 * if voltage falls below 9.0 V.
 */
void test_BatteryMonitor_ShouldTransitionFromLowToCritical(void)
{
    MockAdcBatteryVoltage(10500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_LOW,
        BatteryMonitor_GetState()
    );

    MockAdcBatteryVoltage(8500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_CRITICAL,
        BatteryMonitor_GetState()
    );
}


/*
 * Test 13:
 * CRITICAL should remain active below
 * the 9.5 V recovery threshold.
 */
void test_BatteryMonitor_ShouldRemainCriticalBelow9500mV(void)
{
    MockAdcBatteryVoltage(8500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_CRITICAL,
        BatteryMonitor_GetState()
    );

    MockAdcBatteryVoltage(9200U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_CRITICAL,
        BatteryMonitor_GetState()
    );
}


/*
 * Test 14:
 * Exactly 9.5 V should exit CRITICAL.
 *
 * Because it is still below 11.5 V,
 * the resulting state should be LOW.
 */
void test_BatteryMonitor_ShouldRecoverFromCriticalToLowAt9500mV(void)
{
    MockAdcBatteryVoltage(8500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_CRITICAL,
        BatteryMonitor_GetState()
    );

    MockAdcBatteryVoltage(9500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_LOW,
        BatteryMonitor_GetState()
    );
}


/*
 * Test 15:
 * CRITICAL can recover directly to NORMAL
 * if voltage rises to 11.5 V or more.
 */
void test_BatteryMonitor_ShouldRecoverFromCriticalToNormal(void)
{
    MockAdcBatteryVoltage(8500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_CRITICAL,
        BatteryMonitor_GetState()
    );

    MockAdcBatteryVoltage(12000U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_NORMAL,
        BatteryMonitor_GetState()
    );
}


/*
 * Test 16:
 * OVER_VOLTAGE should remain active while
 * voltage is greater than 15.0 V.
 */
void test_BatteryMonitor_ShouldRemainOverVoltageAbove15000mV(void)
{
    MockAdcBatteryVoltage(16000U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_OVER_VOLTAGE,
        BatteryMonitor_GetState()
    );

    MockAdcBatteryVoltage(15200U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_OVER_VOLTAGE,
        BatteryMonitor_GetState()
    );
}


/*
 * Test 17:
 * OVER_VOLTAGE should recover to NORMAL
 * at exactly 15.0 V.
 */
void test_BatteryMonitor_ShouldRecoverFromOverVoltageAt15000mV(void)
{
    MockAdcBatteryVoltage(16000U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_OVER_VOLTAGE,
        BatteryMonitor_GetState()
    );

    MockAdcBatteryVoltage(15000U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_NORMAL,
        BatteryMonitor_GetState()
    );
}


/*
 * Test 18:
 * If ADC reading fails, previously stored voltage
 * should remain unchanged.
 */
void test_BatteryMonitor_ShouldKeepPreviousVoltageWhenAdcReadFails(void)
{
    BatteryMonitor_UpdateVoltageMv(12500U);

    ADC_Driver_ReadBatteryMv_ExpectAnyArgsAndReturn(false);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL_UINT16(
        12500U,
        BatteryMonitor_GetVoltageMv()
    );

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_NORMAL,
        BatteryMonitor_GetState()
    );
}


/*
 * Test 19:
 * IsLowVoltage should return TRUE when state is LOW.
 */
void test_BatteryMonitor_IsLowVoltageShouldReturnTrueForLowState(void)
{
    MockAdcBatteryVoltage(10500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_TRUE(
        BatteryMonitor_IsLowVoltage()
    );
}


/*
 * Test 20:
 * IsLowVoltage should return TRUE when state
 * is CRITICAL.
 */
void test_BatteryMonitor_IsLowVoltageShouldReturnTrueForCriticalState(void)
{
    MockAdcBatteryVoltage(8500U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_TRUE(
        BatteryMonitor_IsLowVoltage()
    );
}


/*
 * Test 21:
 * IsLowVoltage should return FALSE
 * when state is NORMAL.
 */
void test_BatteryMonitor_IsLowVoltageShouldReturnFalseForNormalState(void)
{
    MockAdcBatteryVoltage(12000U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_FALSE(
        BatteryMonitor_IsLowVoltage()
    );
}


/*
 * Test 22:
 * IsLowVoltage should return FALSE for
 * OVER_VOLTAGE.
 */
void test_BatteryMonitor_IsLowVoltageShouldReturnFalseForOverVoltage(void)
{
    MockAdcBatteryVoltage(16000U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_FALSE(
        BatteryMonitor_IsLowVoltage()
    );
}


/*
 * Test 23:
 * A successful ADC measurement should update
 * the stored voltage.
 */
void test_BatteryMonitor_ShouldStoreSuccessfulAdcMeasurement(void)
{
    MockAdcBatteryVoltage(13250U);

    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL_UINT16(
        13250U,
        BatteryMonitor_GetVoltageMv()
    );
}


/*
 * Test 24:
 * Verify the complete low-voltage hysteresis cycle:
 *
 * NORMAL
 *   ↓ 10.5 V
 * LOW
 *   ↓ 8.5 V
 * CRITICAL
 *   ↓ 10.0 V
 * LOW
 *   ↓ 12.0 V
 * NORMAL
 */
void test_BatteryMonitor_ShouldHandleCompleteLowVoltageCycle(void)
{
    MockAdcBatteryVoltage(10500U);
    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_LOW,
        BatteryMonitor_GetState()
    );


    MockAdcBatteryVoltage(8500U);
    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_CRITICAL,
        BatteryMonitor_GetState()
    );


    MockAdcBatteryVoltage(10000U);
    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_LOW,
        BatteryMonitor_GetState()
    );


    MockAdcBatteryVoltage(12000U);
    BatteryMonitor_MainFunction();

    TEST_ASSERT_EQUAL(
        BATTERY_STATE_NORMAL,
        BatteryMonitor_GetState()
    );
}