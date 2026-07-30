#include "unity.h"

#include "diagnostic_manager.h"

#include "mock_fault_manager.h"
#include "mock_event_logger.h"
#include "mock_battery_monitor.h"


void setUp(void)
{
    DiagnosticManager_Init();
}


void tearDown(void)
{
}


/*
 * Test 1:
 * After initialization, no DTCs should be active.
 */
void test_DiagnosticManager_ShouldStartWithNoActiveDTCs(void)
{
    TEST_ASSERT_EQUAL_UINT32(
        0U,
        DiagnosticManager_GetDTCMask()
    );

    TEST_ASSERT_FALSE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_LOW
        )
    );

    TEST_ASSERT_FALSE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_CRITICAL
        )
    );

    TEST_ASSERT_FALSE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_OVER_VOLTAGE
        )
    );

    TEST_ASSERT_FALSE(
        DiagnosticManager_IsDTCActive(
            DTC_INVALID_VEHICLE_STATE
        )
    );
}


/*
 * Helper:
 * Configure all FaultManager queries.
 */
static void MockFaultStates(bool low,
                            bool critical,
                            bool over_voltage,
                            bool invalid_vehicle_state)
{
    FaultManager_IsFaultActive_ExpectAndReturn(
        FAULT_ID_BATTERY_LOW,
        low
    );

    FaultManager_IsFaultActive_ExpectAndReturn(
        FAULT_ID_BATTERY_CRITICAL,
        critical
    );

    FaultManager_IsFaultActive_ExpectAndReturn(
        FAULT_ID_BATTERY_OVER_VOLTAGE,
        over_voltage
    );

    FaultManager_IsFaultActive_ExpectAndReturn(
        FAULT_ID_INVALID_VEHICLE_STATE,
        invalid_vehicle_state
    );
}


/*
 * Test 2:
 * No active faults should produce no DTCs.
 */
void test_DiagnosticManager_ShouldHaveNoDTCsWhenNoFaultsActive(void)
{
    MockFaultStates(
        false,
        false,
        false,
        false
    );

    DiagnosticManager_MainFunction();

    TEST_ASSERT_EQUAL_UINT32(
        0U,
        DiagnosticManager_GetDTCMask()
    );
}


/*
 * Test 3:
 * Battery LOW fault should activate DTC bit 0.
 */
void test_DiagnosticManager_ShouldSetBatteryLowDTC(void)
{
    MockFaultStates(
        true,
        false,
        false,
        false
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        10500U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_LOW,
        10500U
    );

    DiagnosticManager_MainFunction();

    TEST_ASSERT_TRUE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_LOW
        )
    );

    TEST_ASSERT_EQUAL_UINT32(
        (1UL << 0),
        DiagnosticManager_GetDTCMask()
    );
}


/*
 * Test 4:
 * Battery CRITICAL fault should activate DTC bit 1.
 */
void test_DiagnosticManager_ShouldSetBatteryCriticalDTC(void)
{
    MockFaultStates(
        false,
        true,
        false,
        false
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        8500U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_CRITICAL,
        8500U
    );

    DiagnosticManager_MainFunction();

    TEST_ASSERT_TRUE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_CRITICAL
        )
    );

    TEST_ASSERT_EQUAL_UINT32(
        (1UL << 1),
        DiagnosticManager_GetDTCMask()
    );
}


/*
 * Test 5:
 * Battery over-voltage fault should activate DTC bit 2.
 */
void test_DiagnosticManager_ShouldSetBatteryOverVoltageDTC(void)
{
    MockFaultStates(
        false,
        false,
        true,
        false
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        16000U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_OVER_VOLTAGE,
        16000U
    );

    DiagnosticManager_MainFunction();

    TEST_ASSERT_TRUE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_OVER_VOLTAGE
        )
    );

    TEST_ASSERT_EQUAL_UINT32(
        (1UL << 2),
        DiagnosticManager_GetDTCMask()
    );
}


/*
 * Test 6:
 * Invalid vehicle state should activate DTC bit 3.
 *
 * Note:
 * Your current DiagnosticManager does not log an
 * EventLogger event for this fault.
 */
void test_DiagnosticManager_ShouldSetInvalidVehicleStateDTC(void)
{
    MockFaultStates(
        false,
        false,
        false,
        true
    );

    DiagnosticManager_MainFunction();

    TEST_ASSERT_TRUE(
        DiagnosticManager_IsDTCActive(
            DTC_INVALID_VEHICLE_STATE
        )
    );

    TEST_ASSERT_EQUAL_UINT32(
        (1UL << 3),
        DiagnosticManager_GetDTCMask()
    );
}


/*
 * Test 7:
 * Multiple active faults should set multiple bits.
 */
void test_DiagnosticManager_ShouldSupportMultipleDTCs(void)
{
    MockFaultStates(
        true,
        false,
        true,
        true
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        10500U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_LOW,
        10500U
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        10500U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_OVER_VOLTAGE,
        10500U
    );

    DiagnosticManager_MainFunction();

    TEST_ASSERT_TRUE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_LOW
        )
    );

    TEST_ASSERT_TRUE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_OVER_VOLTAGE
        )
    );

    TEST_ASSERT_TRUE(
        DiagnosticManager_IsDTCActive(
            DTC_INVALID_VEHICLE_STATE
        )
    );

    TEST_ASSERT_EQUAL_UINT32(
        ((1UL << 0) |
         (1UL << 2) |
         (1UL << 3)),
        DiagnosticManager_GetDTCMask()
    );
}


/*
 * Test 8:
 * DTC_BATTERY_LOW should return FALSE if inactive.
 */
void test_DiagnosticManager_ShouldReturnFalseForInactiveBatteryLowDTC(void)
{
    TEST_ASSERT_FALSE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_LOW
        )
    );
}


/*
 * Test 9:
 * DTC_NONE should always return FALSE.
 */
void test_DiagnosticManager_ShouldReturnFalseForDTCNone(void)
{
    TEST_ASSERT_FALSE(
        DiagnosticManager_IsDTCActive(
            DTC_NONE
        )
    );
}


/*
 * Test 10:
 * Unknown DTC values should return FALSE.
 */
void test_DiagnosticManager_ShouldReturnFalseForUnknownDTC(void)
{
    diagnostic_code_t invalid_dtc =
        (diagnostic_code_t)0xFFFFU;

    TEST_ASSERT_FALSE(
        DiagnosticManager_IsDTCActive(
            invalid_dtc
        )
    );
}


/*
 * Test 11:
 * A DTC should clear when the corresponding
 * fault disappears.
 */
void test_DiagnosticManager_ShouldClearBatteryLowDTCAfterFaultClears(void)
{
    /*
     * First cycle: LOW fault appears.
     */
    MockFaultStates(
        true,
        false,
        false,
        false
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        10500U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_LOW,
        10500U
    );

    DiagnosticManager_MainFunction();

    TEST_ASSERT_TRUE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_LOW
        )
    );


    /*
     * Second cycle: fault disappears.
     */
    MockFaultStates(
        false,
        false,
        false,
        false
    );

    DiagnosticManager_MainFunction();

    TEST_ASSERT_FALSE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_LOW
        )
    );

    TEST_ASSERT_EQUAL_UINT32(
        0U,
        DiagnosticManager_GetDTCMask()
    );
}


/*
 * Test 12:
 * A continuously active LOW fault should only
 * generate one event when it first appears.
 */
void test_DiagnosticManager_ShouldLogBatteryLowOnlyWhenFaultFirstAppears(void)
{
    /*
     * First cycle:
     * LOW appears -> event expected.
     */
    MockFaultStates(
        true,
        false,
        false,
        false
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        10500U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_LOW,
        10500U
    );

    DiagnosticManager_MainFunction();


    /*
     * Second cycle:
     * LOW still active.
     *
     * No BatteryMonitor_GetVoltageMv()
     * and no EventLogger_Log() should occur.
     */
    MockFaultStates(
        true,
        false,
        false,
        false
    );

    DiagnosticManager_MainFunction();

    TEST_ASSERT_TRUE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_LOW
        )
    );
}


/*
 * Test 13:
 * If a LOW fault clears and later reappears,
 * it should be logged again.
 */
void test_DiagnosticManager_ShouldLogBatteryLowAgainAfterReappearing(void)
{
    /*
     * First appearance.
     */
    MockFaultStates(
        true,
        false,
        false,
        false
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        10500U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_LOW,
        10500U
    );

    DiagnosticManager_MainFunction();


    /*
     * Fault clears.
     */
    MockFaultStates(
        false,
        false,
        false,
        false
    );

    DiagnosticManager_MainFunction();


    /*
     * Fault appears again.
     */
    MockFaultStates(
        true,
        false,
        false,
        false
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        10400U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_LOW,
        10400U
    );

    DiagnosticManager_MainFunction();

    TEST_ASSERT_TRUE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_LOW
        )
    );
}


/*
 * Test 14:
 * Transition from LOW to CRITICAL should:
 *
 * clear LOW DTC
 * set CRITICAL DTC
 * log CRITICAL event once
 */
void test_DiagnosticManager_ShouldTransitionFromLowToCriticalDTC(void)
{
    /*
     * LOW first.
     */
    MockFaultStates(
        true,
        false,
        false,
        false
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        10500U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_LOW,
        10500U
    );

    DiagnosticManager_MainFunction();


    /*
     * Then CRITICAL.
     */
    MockFaultStates(
        false,
        true,
        false,
        false
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        8500U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_CRITICAL,
        8500U
    );

    DiagnosticManager_MainFunction();

    TEST_ASSERT_FALSE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_LOW
        )
    );

    TEST_ASSERT_TRUE(
        DiagnosticManager_IsDTCActive(
            DTC_BATTERY_CRITICAL
        )
    );
}


/*
 * Test 15:
 * Verify all four DTC bits simultaneously.
 */
void test_DiagnosticManager_ShouldSetAllFourDTCBits(void)
{
    MockFaultStates(
        true,
        true,
        true,
        true
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        8500U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_LOW,
        8500U
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        8500U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_CRITICAL,
        8500U
    );

    BatteryMonitor_GetVoltageMv_ExpectAndReturn(
        8500U
    );

    EventLogger_Log_Expect(
        EVENT_BATTERY_OVER_VOLTAGE,
        8500U
    );

    DiagnosticManager_MainFunction();

    TEST_ASSERT_EQUAL_UINT32(
        0x0FU,
        DiagnosticManager_GetDTCMask()
    );
}