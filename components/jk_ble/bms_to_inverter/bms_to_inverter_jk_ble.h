#pragma once

#include "esphome/core/defines.h"

// NOLINTNEXTLINE
#include "esphome/components/jk_bms_ble/jk_bms_ble.h"
#include "esphome/components/bms_to_inverter/bms_to_inverter.h"

namespace esphome {
namespace jk_ble {

class BmsToInverterJKBle : public bms_to_inverter::BmsToInverter {
 public:
  explicit BmsToInverterJKBle(jk_bms_ble::JkBmsBle *bms) : bms_(bms){};

  void setup() override;
  void update() override;

 protected:
  jk_bms_ble::JkBmsBle *bms_;

  void update_bms_data(bms_to_inverter::BmsData *bms_data) override;
};

}  // namespace jk_ble
}  // namespace esphome
