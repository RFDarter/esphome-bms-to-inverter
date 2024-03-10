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

struct SmaCanMessage0x0351 {
  uint16_t ChargeVoltage;          // U16
  int16_t MaxChargingCurrent;      // S16
  int16_t MaxDischargingCurrent;   // S16
  uint16_t DischargeVoltageLimit;  // U16
};

struct SmaCanMessage0x0355 {
  uint16_t StateOfCharge;         // U16
  uint16_t StateOfHealth;         // U16
  uint16_t StateOfChargeHighRes;  // U16
};

struct SmaCanMessage0x0356 {
  int16_t BatteryVoltage;      // S16
  int16_t BatteryCurrent;      // S16
  int16_t BatteryTemperature;  // S16
};

struct SmaCanMessage0x035A {
  uint32_t AlarmBitmask;    // 32 Bits
  uint32_t WarningBitmask;  // 32 Bits
};

struct SmaCanMessage0x035E {
  char Model[8];
};

struct SmaCanMessage0x035F {
  uint16_t CellChemistry;
  uint8_t HardwareVersion[2];
  uint16_t NominalCapacity;
  uint8_t SoftwareVersion[2];
};

struct SmaCanMessage0x0370 {
  char Manufacturer[8];
};

struct SmaCanMessage0x0373 {
  uint16_t MinCellvoltage;  // v * 1000.0f
  uint16_t MaxCellvoltage;  // v * 1000.0f
  uint16_t MinTemperature;  // v * 273.15f
  uint16_t MaxTemperature;  // v * 273.15f
};

struct SmaCanMessage0x0380 {
  uint8_t BatteryCount;   // number off batterys connnected
  uint8_t CellCount;      // number off cells per battery
  uint8_t ParallelCount;  // number off batterys connnected parallel
  uint8_t SerielCount;    // number off batterys connnected in series
};

}  // namespace bms_to_inverter
}  // namespace esphome
