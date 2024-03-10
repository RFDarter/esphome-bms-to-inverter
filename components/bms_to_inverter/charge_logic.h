#pragma once

#include <stdint.h>  // NOLINT
#include <string>

namespace esphome {
namespace bms_to_inverter {

class BmsToInverter;

enum ChargeStatus {
  CHARGE_STATUS_NO_BMS_CONNECTED,
  CHARGE_STATUS_WAIT,
  CHARGE_STATUS_BULK,
  CHARGE_STATUS_ABSORBTION,
  CHARGE_STATUS_FLOAT,
};

// Conatains the data modified ba the selected charge logic
struct ChargeLogicValues {
  float charge_voltage;
  float max_charge_current;
  float max_discharge_current;
};

class ChargeLogic {
 public:
  ChargeLogic(BmsToInverter *parent) : parent_(parent) {}
  /**
   * Call base in every implementation
   */
  virtual void update_charge_logic_values(ChargeLogicValues *charge_logic_values) = 0;
  virtual void change_charge_status(ChargeStatus charge_status) = 0;
  virtual bool are_essential_values_present() = 0;

  ChargeStatus charge_status_{CHARGE_STATUS_NO_BMS_CONNECTED};

 protected:
  BmsToInverter *parent_{nullptr};
};

}  // namespace bms_to_inverter
}  // namespace esphome
