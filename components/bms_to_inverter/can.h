#pragma once

#include "esphome/components/canbus/canbus.h"

namespace esphome {
namespace bms_to_inverter {

// Slave -> Master
struct CanSlaveToMasterMessage0x170 {
  uint8_t connection_request;
};

// Master -> Slave
struct CanMasterToSlaveMessage0x171 {
  uint8_t connection_response_slave_id;
};

// Slave -> Master || would need to be recives every x seconds or master assumes that slave got disconnected
struct CanSlaveToMasterMessage0x172 {
  uint8_t SlaveId;                // U8
  int16_t MaxChargingCurrent;     // S16
  int16_t MaxDischargingCurrent;  // S16
  uint16_t StateOfCharge;         // U16
  uint16_t StateOfHealth;         // U16
  int16_t Current;                // S16
  uint16_t NominalCapacity;       // U16
};

// Pylon 1.2 | Pylon + | SMA | Victron
struct CanMessage0x0351 {
  uint16_t ChargeVoltage;          // U16
  int16_t MaxChargingCurrent;      // S16
  int16_t MaxDischargingCurrent;   // S16
  uint16_t DischargeVoltageLimit;  // U16
};

// Pylon 1.2 | Pylon + | SMA | Victron
struct CanMessage0x0355 {
  uint16_t StateOfCharge;         // U16
  uint16_t StateOfHealth;         // U16
  uint16_t StateOfChargeHighRes;  // U16
};

// Pylon 1.2 | Pylon + | SMA | Victron
struct CanMessage0x0356 {
  int16_t BatteryVoltage;      // S16
  int16_t BatteryCurrent;      // S16
  int16_t BatteryTemperature;  // S16
  uint16_t ChargeCycles;       // Sofar inverter
};

// SMA | Victron
/* according to the can protocol these are the error bits
Bit 0
00000100 high voltage
00010000 low voltage
01000000 high temp
Bit 1
00000001 low temp
00000100 high charge temp
00010000 low charge temp
01000000 high discharge current
Bit 2
00000001 high charge current
00000100 contactor
00010000 short circute
01000000 bms internal
*/
struct CanMessage0x035A {
  uint32_t AlarmBitmask;    // 32 Bits
  uint32_t WarningBitmask;  // 32 Bits
};

// Pylon 1.2 | Pylon +
/*
Byte 0
01000000  Discharge enabled
10000000  Charge enabled
*/
struct CanMessage0x035C {
  uint16_t RequestBitmask;
};

// SMA | Pylon 1.2 | Pylonn + | Victron
struct CanMessage0x035E {
  char Model[8];
};

// SMA
struct CanMessageSma0x035F {
  uint16_t CellChemistry;
  uint8_t SoftwareVersion[2];
  uint16_t NominalCapacity;
  uint8_t HardwareVersion[2];
};

// Victron
struct CanMessageVictron0x035F {
  uint16_t ProductId;
  uint8_t HardwareVersion[2];
  uint16_t NominalCapacity;
  uint8_t SoftwareVersion[2];
};

// Pylon 1.2 | Pylon +
/*
Byte 0
00000010  high voltage
00000100  low voltage
00001000  hight temp
00010000  low temp
01000000  charge overcurrent
Byte 1
00000001  discharge overcurrent
00001000  internal bms error
00010000  cell imbalance
*/
struct CanMessage0x0359 {
  uint16_t AlarmBitmask;
  uint16_t WarningBitmask;
  uint8_t ParallelCount;
  uint8_t Unknown1;
  uint8_t Unknown2;
  uint8_t DipSwitches;
};

// Pylon 1.2 | Pylon +
struct CanMessage0x070 {
  int16_t MaxTemperature;
  int16_t MinTemperature;
  int16_t MaxCellVoltage;
  int16_t MinCellVoltage;
};

// Pylon 1.2 | Pylon +
struct CanMessage0x0371 {
  uint16_t MaxTemperaturSensorId;
  uint16_t MinTemperaturSensorId;
  uint16_t MaxVoltageCellId;
  uint16_t MinVoltageCellId;
};

// Victron
struct CanMessage0x0372 {
  uint16_t BatteryModulesCount;
  uint16_t BatteryModulesBlockChargingCount;
  uint16_t BatteryModulesBlockDischargingCount;
  uint16_t BatteryModulesOfflineCount;
};

// Victron
struct CanMessage0x0373 {
  uint16_t MinCellvoltage;  // v * 1000.0f
  uint16_t MaxCellvoltage;  // v * 1000.0f
  uint16_t MinTemperature;  // v * 273.15f
  uint16_t MaxTemperature;  // v * 273.15f
};

// Vicron
struct CanMessage0x0374 {  // Min voltage cell id as ASCII ([0] = 49 would be cell id 1)
  char MinVoltageCellId[8];
};

// Vicron
struct CanMessage0x0375 {  // Max voltage cell id as ASCII ([0] = 49 would be cell id 1)
  char MaxVoltageCellId[8];
};

// Vicron
struct CanMessage0x0376 {  // Min Temperatur sensor id as ASCII ([0] = 49 would be temp sensor id 1)
  char MinTempertaurSensorId[8];
};

// Vicron
struct CanMessage0x0377 {  // Max Temperatur sensor id as ASCII ([0] = 49 would be temp sensor id 1)
  char MaxTempertaurSensorId[8];
};

// Pylon + | Victron (Sol-Ark, Luxpower)
struct CanMessage0x0379 {
  uint16_t NominalCapacity;
  uint16_t Unknown1;
  uint16_t Unknown2;
  uint16_t Unknown3;
};

// Victron
struct CanMessage0x0382 {
  char ProduktIdentification[8];
};

}  // namespace bms_to_inverter
}  // namespace esphome
