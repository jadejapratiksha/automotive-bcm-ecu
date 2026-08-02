#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include <stdint.h>

/*
 * CAN message identifiers
 */
#define CAN_ID_IGNITION_COMMAND    (0x100U)
#define CAN_ID_BCM_STATUS          (0x200U)

/*
 * Ignition command frame: CAN ID 0x100
 *
 * Byte 0:
 *   0 = Vehicle OFF
 *   1 = Vehicle ACCESSORY
 *   2 = Vehicle RUNNING
 */
#define CAN_IGNITION_COMMAND_DLC   (1U)

#define CAN_IGNITION_OFF           (0U)
#define CAN_IGNITION_ACCESSORY     (1U)
#define CAN_IGNITION_RUNNING       (2U)

/*
 * BCM status frame: CAN ID 0x200
 *
 * Byte 0: Vehicle state
 * Byte 1: Front-left door state
 * Byte 2: Battery voltage low byte
 * Byte 3: Battery voltage high byte
 * Byte 4: Active fault mask byte 0
 * Byte 5: Active fault mask byte 1
 * Byte 6: Active fault mask byte 2
 * Byte 7: Active fault mask byte 3
 */
#define CAN_BCM_STATUS_DLC              (8U)

#define CAN_BCM_STATUS_VEHICLE_BYTE     (0U)
#define CAN_BCM_STATUS_DOOR_BYTE        (1U)
#define CAN_BCM_STATUS_BATTERY_LSB      (2U)
#define CAN_BCM_STATUS_BATTERY_MSB      (3U)
#define CAN_BCM_STATUS_FAULT_BYTE_0     (4U)
#define CAN_BCM_STATUS_FAULT_BYTE_1     (5U)
#define CAN_BCM_STATUS_FAULT_BYTE_2     (6U)
#define CAN_BCM_STATUS_FAULT_BYTE_3     (7U)

#endif