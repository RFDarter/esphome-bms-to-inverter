#pragma once

#include <string>
#include "charge_logic.h"

namespace esphome {
namespace bms_to_inverter {

class ChargeLogicRebulkVoltage : public ChargeLogic {
 public:
  ChargeLogicRebulkVoltage(BmsToInverter *parent) : ChargeLogic(parent) {}
  void update_charge_logic_values(ChargeLogicValues *charge_logic_values) override;
  void change_charge_status(ChargeStatus charge_status) override;
  bool are_essential_values_present() override;

 protected:
  uint32_t timestamp_started_absorbtion_;
};

}  // namespace bms_to_inverter
}  // namespace esphome
